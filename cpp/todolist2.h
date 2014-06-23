/**
 * (c) 2014 Pat Morin, Released under a Creative Commons Attribution (CC-BY)
 *     license.
 *
 * todolist2.h : A variant of the top-down skiplist
 *
 * In this variant, the constructor has to be supplied with a value, max0
 * that is larger than any element that will ever be stored in the structure.
 * This value is then used in a sentinel that eliminates a null-pointer check
 * within the inner loop of find(x).
 *
 * I was hopeful for this, but preliminary experiments suggest that it doesn't
 * really speed things up.
 */
#ifndef FASTWS_TODOLIST2_H_
#define FASTWS_TODOLIST2_H_

#include <cmath>
#include <cstring>
#include <cstdlib>
#include <climits>
#include <cassert>

#include <iostream>
using namespace std;

namespace fastws {

template<class T>
class TodoList2 {
protected:
	struct NP;

	struct Node {
		T x;          // data
		Node *next[]; // a stack of next pointers
	};

	int k;    // there are k+1 lists numbered 0,...,k
	int *n;   // n[i] is the size of the i'th list
	Node *sentinel; // sentinel-next[i] is the first element of list i
	Node *sentinel2;

	// parameters used to determine lists sizes
	double eps;
	int n0max;
	int *a;

	T t_max;

	// FIXME: for profiling information
	int *rebuild_freqs;

	void init(T *data, int n);
	void rebuild();
	void rebuild(int i);

	void sanity();

	Node *newNode();
	void deleteNode(Node *u);

public:
	TodoList2(T *data, int n0, double eps0, T max0);
	virtual ~TodoList2();
	T find(T x);
	bool add(T x);
	int size() {
		return n[k];
	}

	void printOn(std::ostream &out);
};

template<class T>
TodoList2<T>::TodoList2(T *data, int n0, double eps0, T max0) {
	eps = eps0;
	t_max = max0;

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
void TodoList2<T>::init(T *data, int n0) {

	// Compute critical values depending on epsilon and n
	n0max = ceil(2. / eps);
	n0max = 2;
	// cout << "n0max = " << n0max << endl;
	k = 1 + max(0.0, ceil(log(n0) / log(2-eps)));

	n = new int[k + 1]();
	// cout << "k = " << k << endl;
	n[k] = n0;
	sentinel = newNode();
	sentinel2 = newNode();
	sentinel2->x = t_max;
	Node *prev = sentinel;
	for (int i = 0; i < n0; i++) {
		Node *u = newNode();
		u->x = data[i];
		prev->next[k] = u;
		prev = u;
	}
	prev->next[k] = sentinel2;
	rebuild(k);
}

template<class T>
typename TodoList2<T>::Node* TodoList2<T>::newNode() {
	Node *u = (Node *) malloc(sizeof(Node) + (k + 1) * sizeof(Node*));
	memset(u->next, '\0', (k + 1) * sizeof(Node*));
	return u;
}

template<class T>
void TodoList2<T>::deleteNode(Node *u) {
	free(u);
}

template<class T>
void TodoList2<T>::rebuild() {
	// time to rebuild --- free everything and start over
	// TODO: Put some padding in so we only do this O(loglog n) times
	T *data = new T[n[k]+1];
	Node *prev = sentinel;
	Node *u = sentinel->next[k];
	for (int j = 0; j <= n[k]; j++) {
		data[j] = u->x;
		deleteNode(prev);
		prev = u;
		u = u->next[k];
	}
	deleteNode(prev);
	int enn = n[k];
	delete[] n;
	init(data, enn);
	delete[] data;
}


template<class T>
void TodoList2<T>::rebuild(int i) {

	rebuild_freqs[i]++;

	for (int j = i - 1; j >= 0; j--) {
		// populate L_j using L_{j+1}
		n[j] = -1;  // start at -1 to make up for sentinel2
		Node *u = sentinel->next[j + 1];
		Node *prev = sentinel;
		bool skipped = false;
		while (u != NULL) {
			if (skipped || u == sentinel2) {
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
T TodoList2<T>::find(T x) {
	Node *u = sentinel;
	for (int i = 0; i <= k; i++) {
		if (u->next[i]->x < x)
			u = u->next[i];
	}
	Node *w = u->next[k];
	return (w == sentinel2) ? (T)NULL : w->x;
}

template<class T>
bool TodoList2<T>::add(T x) {
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
TodoList2<T>::~TodoList2() {
	delete[] n;
	delete[] a;
	delete[] rebuild_freqs;
	Node *prev = sentinel;
	while (prev != NULL) {
		Node *u = prev->next[k];
		prev->next[k] = NULL;
		deleteNode(prev);
		prev = u;
	}
}

template<class T>
void TodoList2<T>::sanity() {
	assert(n[0] <= n0max);
	for (int i = 0; i <= k; i++) {
		Node *u = sentinel;
		for (int j = 0; j < n[i]; j++) {
			assert(u == sentinel || u->x < u->next->x);
			u = u->next[i];
		}
		assert(u->next[i] == sentinel2);
	}
}

template<class T>
void TodoList2<T>::printOn(std::ostream &out) {
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
			assert(u == sentinel2);
		}
		cout << " n(" << i << ") = " << n[i]
		     << " (rebuilt " << rebuild_freqs[i] << " times)" << endl;
	}
}

template<class T>
ostream& operator<<(ostream &out, TodoList2<T> &sl) {
	sl.printOn(out);
	return out;
}

} // fastws namespace

#endif // FASTWS_TODOLIST2_H_
