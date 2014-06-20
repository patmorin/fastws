/*
 * SplayTree.h
 *
 *  Created on: 2011-11-28
 *      Author: morin
 */

#ifndef SplayTree_H_
#define SplayTree_H_

#include <climits>

#include "BinarySearchTree.h"

namespace ods {

template<class Node, class T>
class SplayTree : public BinarySearchTree<Node, T> {
protected:
	using BinaryTree<Node>::r;
	using BinaryTree<Node>::nil;
	using BinarySearchTree<Node,T>::null;
	using BinarySearchTree<Node,T>::n;
	using BinarySearchTree<Node,T>::findLast;
	using BinarySearchTree<Node,T>::rotateLeft;
	using BinarySearchTree<Node,T>::rotateRight;
	using BinarySearchTree<Node,T>::splice;

	void splay(Node *u);

public:
	SplayTree();
	virtual ~SplayTree();
	virtual bool add(T x);
	virtual bool remove(T x);
	virtual T find(T x);
};

template<class T>
class SplayTree1 : public SplayTree<BSTNode1<T>, T> { };

template<class Node, class T>
SplayTree<Node, T>::SplayTree() {
	// nothing to do
}

template<class Node, class T>
bool SplayTree<Node, T>::add(T x) {
	Node *u = new Node;
	u->x = x;
	if (BinarySearchTree<Node,T>::add(u)) {
		splay(u);
		return true;
	}
	return false;
}

template<class Node, class T>
SplayTree<Node, T>::~SplayTree() {
	// nothing to do
}

template<class Node, class T>
void SplayTree<Node, T>::splay(Node *u) {
    while (u != r) {
        if (u->parent->parent == nil) { // u is a child of the root
            if (u == u->parent->left) { // zig
                rotateRight(u->parent);
            } else { // zag
                rotateLeft(u->parent);
            }
        } else if (u == u->parent->left) {
            if (u->parent == u->parent->parent->left) { // zig-zig
                rotateRight(u->parent->parent);
                rotateRight(u->parent);
            } else { // zig-zag
                rotateRight(u->parent);
                rotateLeft(u->parent);
            }
        } else {
            if (u->parent == u->parent->parent->right) { // zag-zag
                rotateLeft(u->parent->parent);
                rotateLeft(u->parent);
            } else { // zag-zig
                rotateLeft(u->parent);
                rotateRight(u->parent);
            }
        }
    }
}

template<class Node, class T>
T SplayTree<Node, T>::find(T x) {
        Node *w = r;
        Node *prev = nil;
        Node *z = nil;
        while (w != nil) {
            prev = w;
            if (x < w->x) {
                z = w;
                w = w->left;
            } else if (x > w->x) {
                w = w->right;
            } else {
                splay(w);
                return w->x;
            }
        }
        if (prev != nil) splay(prev);
        if (z == nil) return (T)NULL; // FIXME: not portable
        return z->x;
}


template<class Node, class T>
bool SplayTree<Node, T>::remove(T x) {
	Node *u = findLast(x);
	if (u != nil && compare(u->x, x) == 0) {
		splice(u);
		delete u;
		return true;
	}
	return false;
}



} /* namespace ods */
#endif /* SplayTree_H_ */
