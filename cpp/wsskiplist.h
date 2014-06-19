/**
 * (c) 2014 Pat Morin, Released under a Creative Commons Attribution (CC-BY) license.
 *
 * wsskiplist.h : An implementation of the working-set skiplist structure
 *
 * This particular implementation is a space hog.  Every element in the structure has its own array
 * of length k=Theta(log n)$ that is used to store its previous and next pointers.  This avoids the
 * allocating and freeing of nodes when nodes are promoted or levels are rebuilt.
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
		int w;   // the working-set number (not always correct)
		T x;     // data

		// these maintain a global queue ordered by working-set number
		Node *qnext;
		Node *qprev;

		Node *next[]; // a stack of next pointers
	};

	Node *sentinel;
	int *n;
	int k;

	int *a;
	int *b;

	void init(T *data, int n);
	void rebuild(int k);

	Node *newNode();

	// FIXME: integer only
	int cmp(const T &a, const T &b) {
		return a - b;
	}

public:
	WSSkiplist(T *data, int n0);
	virtual ~WSSkiplist();
	T find(T x);

	void printOn(std::ostream &out);
};

template<class T>
WSSkiplist<T>::WSSkiplist(T *data, int n0) {
	init(data, n0);
}

template<class T>
void WSSkiplist<T>::init(T *data, int n0) {

	// Compute critical values depending on epsilon
	double eps = 0.5;
	k = 2.0 + log(n0) / log(1+eps);
    a = new int[k+1];
    b = new int[k+1];
    for (int i = 0; i <= k; i++) {
    	a[i] = pow(2.-eps, i);
    	b[i] = pow(2.-eps/2, i);
    	cout << a[i] << "," << b[i] << endl;
    }

	n = new int[k+1];
	n[k] = n0;
	sentinel = newNode();
	sentinel->x = -1; // FIXME: non-negative integer only
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
	memset(u->next, (k + 1) * sizeof(Node*), '\0');
	return u;
}

template<class T>
void WSSkiplist<T>::rebuild(int k) {
	// compute working-set numbers of relevant nodes
	Node *u = sentinel->qnext;
	int wmax = a[k-1];
	for (int i = 0; i < wmax; i++) {
		u->w = i;
		u = u->qnext;
	}

	for (int i = k - 1; i >= 0; i--) {
		n[i] = 0;
		Node *u = sentinel->next[i + 1];
		Node *prev = sentinel;
		int w = a[i];
		cout << "i = " << i << ", w = " << w << endl;
		bool skipped = false;
		while (u != NULL) {
			if (skipped || u->w <= w) {
				prev->next[i] = u;
				prev = u;
				n[i]++;
				skipped = false;
			} else {
				skipped = true;
			}
			u = u->next[i + 1];
		}
	}

	// reset all working-set numbers
	u = sentinel->qnext;
	for (int i = 0; i < wmax; i++) {
		u->w = INT_MAX;
		u = u->qnext;
	}

}

template<class T>
T WSSkiplist<T>::find(T x) {
	cout << "Searching for " << x << ":";
	Node *stack[50]; // FIXME: fixed upper bound
	Node *u = sentinel;
	int c = -1, i = 0;
	while (u->next[i] != NULL && (c = cmp(u->next[i]->x, x)) < 0)
		u = u->next[i];
	stack[i] = u;
	cout << u->x << ",";
	if (c != 0) {
		for (i = 1; i <= k; i++) {
			if (u->next[i] != NULL && (c = cmp(u->next[i]->x, x)) < 0)
				u = u->next[i];
			stack[i] = u;
			if (c == 0)
				break;
			cout << u->x << ",";
		}
		cout << endl;
	}

	// Search is done: we're going to return w->x
	Node *w = u->next[i];
	if (w == NULL)
		return (T) NULL;  // FIXME: not portable

	// Add w to lists L_0,...,L_{i-1}
	while (i > 0) {
		i--;
		n[i]++;
		w->next[i] = stack[i]->next[i];
		stack[i]->next[i] = w;
	}

	// move w to the front of the front of the working-set queue
	w->qnext->qprev = w->qprev;
	w->qprev->qnext = w->qnext;
	w->qprev = sentinel;
	w->qnext = sentinel->qnext;
	sentinel->qnext->qprev = w;
	sentinel->qnext = w;

	return w->x;
}

template<class T>
WSSkiplist<T>::~WSSkiplist() {
	Node *u = sentinel;
	while (u != NULL) {
		Node *prev = u;
		u = u->next[k];
		delete prev;
	}
}

template<class T>
void WSSkiplist<T>::printOn(std::ostream &out) {
	cout << "WSSkiplist: n = " << n << ", k = " << k << endl;
	for (int i = 0; i <= k; i++) {
		cout << "L(" << i << "): ";
		Node *u = sentinel->next[i];
		while (u != NULL) {
			cout << u->x << ",";
			u = u->next[i];
		}
		cout << " n(" << i << ") = " << n[i] << endl;
	}
	cout << "q = ";
	Node *u = sentinel->qnext;
	while (u != sentinel) {
		cout << u->x << ",";
		u = u->qnext;
	}
	cout << endl;
}

template<class T>
ostream& operator<<(ostream &out, WSSkiplist<T> &sl) {
	sl.printOn(out);
	return out;

}

}

#endif // FASTWS_WSSKIPLIST_H_
