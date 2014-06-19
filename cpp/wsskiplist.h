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
#ifndef FASTWS_WSSKIPLIST_H_
#define FASTWS_WSSKIPLIST_H_

#include <cmath>
#include <cstring>
#include <cstdlib>
#include <climits>
#include <cassert>

namespace fastws {

/**
 * A dictionary with the working-set property.
 */
template<class T>
class WSSkiplist {
protected:
	struct NP;

	struct Node {
		int w; // the working-set number (not always correct)
		T x;   // data

		// these maintain a global queue ordered by working-set number
		Node *qnext;
		Node *qprev;

		Node *next[]; // a stack of next pointers
	};

	int k;    // there are k+1 lists numbered 0,...,k
	int *n;   // n[i] is the size of the i'th list
	Node *sentinel; // sentinel-next[i] is the first element of list i

	// parameters used to determine lists sizes
	double eps;
	int n0max;
	int *a;
	int *b;

	// FIXME: for profiling information
	int *rebuild_freqs;

	void init(T *data, int n);
	void rebuild(int i);

	void sanity();

	Node *newNode();
	void deleteNode(Node *u);

	// FIXME: integer only
	int (*cmp)(const T &a, const T &b);

public:
	WSSkiplist(T *data, int n0, int (*cmp0)(const T&, const T&),
			double eps0);
	virtual ~WSSkiplist();
	T find(T x);
	int size() {
		return n[k];
	}

	void printOn(std::ostream &out);
};

template<class T>
WSSkiplist<T>::WSSkiplist(T *data, int n0, int (*cmp0)(const T&, const T&),
	double eps0) {
	eps = eps0;
	cmp = cmp0;
	init(data, n0);
}

template<class T>
void WSSkiplist<T>::init(T *data, int n0) {

	// Compute critical values depending on epsilon
	n0max = ceil(2. / eps);
	cout << "n0max = " << n0max << endl;
	double base_a = 2.0-eps;
	double base_b = 2.0-eps/2;
	k = ceil(log(n0) / log(base_a));
	a = new int[k + 1];
	b = new int[k + 1];
	int offset = ceil(log(n0max)/log(base_b))-1;
	k -= offset;
	for (int i = 0; i <= k; i++) {
		a[i] = pow(base_a, i+offset);
		b[i] = pow(base_b, i+offset);
		cout << "a[" << i << "]=" << a[i] << ", b[" << i << "]=" << b[i]
				<< endl;
	}

	rebuild_freqs = new int[k + 1]();

	n = new int[k + 1]();

	n[k] = n0;
	sentinel = newNode();
	sentinel->x = -1; // FIXME: non-negative integer only
	sentinel->qnext = sentinel->qprev = sentinel;
	Node *prev = sentinel;
	for (int i = 0; i < n0; i++) {
		Node *u = newNode();
		u->x = data[i];
		prev->next[k] = u;
		u->qprev = prev;
		prev->qnext = u;
		prev = u;
	}
	prev->qnext = sentinel; // the queue is a circular list
	rebuild(k);
}

template<class T>
typename WSSkiplist<T>::Node* WSSkiplist<T>::newNode() {
	Node *u = (Node *) malloc(sizeof(Node) + (k + 1) * sizeof(Node*));
	u->qnext = u->qprev = NULL;
	u->w = INT_MAX;
	memset(u->next, '\0', (k + 1) * sizeof(Node*));
	return u;
}

template<class T>
void WSSkiplist<T>::deleteNode(Node *u) {
	free(u);
}

template<class T>
void WSSkiplist<T>::rebuild(int i) {

	rebuild_freqs[i]++;

	// compute working-set numbers of relevant nodes
	Node *u = sentinel->qnext;
	int wmax = a[i - 1];
	for (int t = 0; t <= wmax; t++) {
		u->w = t + 1;
		u = u->qnext;
	}

	for (int j = i - 1; j >= 0; j--) {
		// populate L_j using L_{j+1}
		n[j] = 0;
		u = sentinel->next[j + 1];
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

	// reset all working-set numbers
	u = sentinel->qnext;
	for (int t = 0; t < wmax; t++) {
		u->w = INT_MAX;
		u = u->qnext;
	}

}

template<class T>
T WSSkiplist<T>::find(T x) {
	Node *blech[50]; // FIXME: fixed upper bound
	Node *u = sentinel;
	int c = -1, i = 0;
	while (u->next[i] != NULL && (c = cmp(u->next[i]->x, x)) < 0)
		u = u->next[i];
	blech[i] = u;
	if (c != 0) {
		for (i = 1; i <= k; i++) {
			if (u->next[i] != NULL && (c = cmp(u->next[i]->x, x)) < 0)
				u = u->next[i];
			blech[i] = u;
			if (c == 0)
				break;
		}
	}

	// Search is done: we're going to return w->x
	i = i > k ? k : i;
	Node *w = u->next[i];
	if (w == NULL)
		return (T) NULL;  // FIXME: not portable

	// Add w to lists L_0,...,L_{i-1}
	while (i > 0) {
		i--;
		if (blech[i]->next[i] != w) {
			n[i]++;
			w->next[i] = blech[i]->next[i];
			blech[i]->next[i] = w;
		}
	}

	// move w to the front of the working-set queue
	w->qnext->qprev = w->qprev;
	w->qprev->qnext = w->qnext;
	w->qprev = sentinel;
	w->qnext = sentinel->qnext;
	sentinel->qnext->qprev = w;
	sentinel->qnext = w;

	// check for rebuild
	if (n[0] > n0max) {
		for (i = 0; n[i] > b[i]; i++)
			;
		rebuild(i);
	}

	return w->x;
}

template<class T>
WSSkiplist<T>::~WSSkiplist() {
	Node *prev = sentinel;
	while (prev != NULL) {
		Node *u = prev->next[k];
		prev->next[k] = NULL;
		deleteNode(prev);
		prev = u;
	}
}

template<class T>
void WSSkiplist<T>::sanity() {
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
void WSSkiplist<T>::printOn(std::ostream &out) {
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
	if (n[k] <= max_print) {
		cout << "q = ";
		Node *u = sentinel->qnext;
		while (u != sentinel) {
			cout << u->x << ",";
			u = u->qnext;
		}
		cout << endl;
	}
}

template<class T>
ostream& operator<<(ostream &out, WSSkiplist<T> &sl) {
	sl.printOn(out);
	return out;
}

} // fastws namespace

#endif // FASTWS_WSSKIPLIST_H_
