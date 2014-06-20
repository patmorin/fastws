/**
 * (c) 2014 Pat Morin, Released under a Creative Commons Attribution (CC-BY)
 *     license.
 *
 * wsskiplist.h : An implementation of the working-set skiplist structure
 *
 * This particular implementation is a space hog.  Every element in the
 * structure has its own array of length k=Theta(log n)$ that is used to
 * store its previous and next pointers.  This avoids the allocating and
 * freeing of nodes when nodes are promoted or levels are rebuilt.
 */
#ifndef FASTWS_TOPSKIPLIST_H_
#define FASTWS_TOPSKIPLIST_H_

#include <cmath>
#include <cstring>
#include <cstdlib>
#include <climits>
#include <cassert>

#include <iostream>
using namespace std;

namespace fastws {

/**
 * A dictionary with the working-set property.
 */
template<class T>
class TopSkiplist {
protected:
	struct NP;

	struct Node {
		int w;        // the working-set number (not always correct)
		T x;          // data
		Node *next[]; // a stack of next pointers
	};

	int k;    // there are k+1 lists numbered 0,...,k
	int *n;   // n[i] is the size of the i'th list
	Node *sentinel; // sentinel-next[i] is the first element of list i

	// parameters used to determine lists sizes
	double eps;
	int n0max;
	int *a;

	// FIXME: for profiling information
	int *rebuild_freqs;

	void init(T *data, int n);
	void rebuild();
	void rebuild(int i);

	void sanity();

	Node *newNode();
	void deleteNode(Node *u);

public:
	TopSkiplist(T *data, int n0, double eps0);
	virtual ~TopSkiplist();
	T find(T x);
	bool add(T x);
	int size() {
		return n[k];
	}

	void printOn(std::ostream &out);
};

template<class T>
TopSkiplist<T>::TopSkiplist(T *data, int n0, double eps0) {
	eps = eps0;

	int kmax = 100; // FIXME: potential limitation here
	rebuild_freqs = new int[kmax+1]();
	double base_a = 2.0-eps;
	a = new int[kmax+1];
	//int offset = ceil(log(n0max)/log(base_a))-1;
	int offset = 0;
	k -= offset;
	for (int i = 0; i <= kmax; i++) {
		a[i] = pow(base_a, i+offset);
		// cout << "a[" << i << "]=" << a[i] << endl;
	}

	init(data, n0);
}

template<class T>
void TopSkiplist<T>::init(T *data, int n0) {

	// Compute critical values depending on epsilon and n
	n0max = ceil(2. / eps);
	// cout << "n0max = " << n0max << endl;
	k = 1 + max(0.0, ceil(log(n0) / log(2-eps)));

	n = new int[k + 1]();

	n[k] = n0;
	sentinel = newNode();
	Node *prev = sentinel;
	for (int i = 0; i < n0; i++) {
		Node *u = newNode();
		u->x = data[i];
		prev->next[k] = u;
		prev = u;
	}
	rebuild(k);
}

template<class T>
typename TopSkiplist<T>::Node* TopSkiplist<T>::newNode() {
	Node *u = (Node *) malloc(sizeof(Node) + (k + 1) * sizeof(Node*));
	u->w = INT_MAX;
	memset(u->next, '\0', (k + 1) * sizeof(Node*));
	return u;
}

template<class T>
void TopSkiplist<T>::deleteNode(Node *u) {
	free(u);
}

template<class T>
void TopSkiplist<T>::rebuild() {
	// time to rebuild --- free everything and start over
	// TODO: Put some padding in so we only do this O(loglog n) times
	T *data = new T[n[k]];
	Node *prev = sentinel;
	Node *u = sentinel->next[k];
	for (int j = 0; j < n[k]; j++) {
		data[j] = u->x;
		deleteNode(prev);
		prev = u;
		u = u->next[k];
	}
	deleteNode(prev);
	init(data, n[k]);
}


template<class T>
void TopSkiplist<T>::rebuild(int i) {

	rebuild_freqs[i]++;

	for (int j = i - 1; j >= 0; j--) {
		// populate L_j using L_{j+1}
		n[j] = 0;
		Node *u = sentinel->next[j + 1];
		Node *prev = sentinel;
		int w = a[j];
		bool skipped = false;
		while (u != NULL) {
			if (skipped || u->w <= w) {
				prev->next[j] = u;
				prev = u;
				n[j]++;
				skipped = false;
			} else {
				skipped = true;
			}
			u = u->next[j + 1];
		}
		prev->next[j] = NULL;
	}

}

template<class T>
T TopSkiplist<T>::find(T x) {
	Node *u = sentinel;
	int i = 0;
	while (u->next[i] != NULL && u->next[i]->x < x)
		u = u->next[i];
	for (i = 1; i <= k; i++) {
		if (u->next[i] != NULL && u->next[i]->x < x)
			u = u->next[i];
	}
	Node *w = u->next[k];
	return (w == NULL) ? (T)NULL : w->x;
}

template<class T>
bool TopSkiplist<T>::add(T x) {
	// do a search for x and keep track of the search path
	Node *path[50]; // FIXME: hard upper-bound
	Node *u = sentinel;
	int i = 0;
	while (u->next[i] != NULL && u->next[i]->x < x)
		u = u->next[i];
	path[i] = u;
	for (i = 1; i <= k; i++) {
		if (u->next[i] != NULL && u->next[i]->x < x)
			u = u->next[i];
		path[i] = u;
	}

	// check if x is already here and, if so, abort
	Node *w = u->next[k];
	if (w != NULL && w->x == x)
		return false;

	// insert x everywhere along the search path
	w = newNode();
	w->x = x;
	for (i = k; i >= 0; i--) {
		w->next[i] = path[i]->next[i];
		path[i]->next[i] = w;
		n[i]++;
	}

	// check if we need to add another level on the bottom
	if (n[k] > a[k])
		rebuild();

	// do partial rebuilding, if necessary
	if (n[0] > n0max) {
		for (i = 1; n[i] > a[i]; i++);
		assert(i <= k);
		rebuild(i);
	}
	return true;
}

template<class T>
TopSkiplist<T>::~TopSkiplist() {
	Node *prev = sentinel;
	while (prev != NULL) {
		Node *u = prev->next[k];
		prev->next[k] = NULL;
		deleteNode(prev);
		prev = u;
	}
}

template<class T>
void TopSkiplist<T>::sanity() {
	assert(n[0] <= n0max);
	for (int i = 0; i <= k; i++) {
		Node *u = sentinel;
		for (int j = 0; j < n[i]; j++) {
			assert(u == sentinel || u->x < u->next->x);
			assert(u->w == INT_MAX);
			u = u->next[i];
		}
		assert(u->next[i] == NULL);
	}
}

template<class T>
void TopSkiplist<T>::printOn(std::ostream &out) {
	const int max_print = 50;
	cout << "WSSkiplist: n = " << n[k] << ", k = " << k << endl;
	for (int i = 0; i <= k; i++) {
		cout << "L(" << i << "): ";
		if (n[k] <= max_print) {
			Node *u = sentinel->next[i];
			for (int j = 0; j < n[i]; j++) {
				cout << u->x << ",";
				u = u->next[i];
			}
			assert(u == NULL);
		}
		cout << " n(" << i << ") = " << n[i]
		     << " (rebuilt " << rebuild_freqs[i] << " times)" << endl;
	}
}

template<class T>
ostream& operator<<(ostream &out, TopSkiplist<T> &sl) {
	sl.printOn(out);
	return out;
}

} // fastws namespace

#endif // FASTWS_TOPSKIPLIST_H_