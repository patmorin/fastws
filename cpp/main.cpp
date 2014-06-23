#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <string>
#include <algorithm>
#include <iterator>
using namespace std;

#include "SkiplistSSet.h"
#include "Treap.h"
#include "SplayTree.h"
#include "RedBlackTree.h"
#include "wsskiplist.h"
#include "todolist.h"
#include "todolist2.h"

// A silly class to use for testing more expensive comparisons
template<size_t len>
class DumbString {
protected:
	char data[len];

public:
	DumbString() {
		for (int i = 0; i < len; i++) {
			data[i] = "CGTA"[rand()%4];
		}
	}
	DumbString(void* x) {
		for (int i = 0; i < len; i++) {
			data[i] = (char)(long)x;
		}
	}
	bool operator <(const DumbString &other) {
		return strncmp(data, other.data, len) < 0;
	}
	bool operator >(const DumbString &other) {
		return strncmp(data, other.data, len) > 0;
	}
	bool operator ==(const DumbString &other) {
		return strncmp(data, other.data, len) == 0;
	}
	void printOn(ostream &out) {
		out << string(data, len);
	}
	operator int() const {
		return (int)data[0];
	}

};


template<size_t len>
ostream& operator<<(ostream &out, DumbString<len> &ds) {
	ds.printOn(out);
	return out;
}

template<size_t len>
DumbString<len> gen_dumbstring() {
	return DumbString<len>();
}

// Compare the results of performing the same operations on two dictionaries
template<class Dict1, class Dict2>
void test_dicts(Dict1 &d1, Dict2 &d2, int n) {
	srand(1);
	for (int i = 0; i < n; i++) {
		int x = rand() % (5*n);
		assert(d1.add(x) == d2.add(x));
	}

	for (int i = 0; i < 5*n; i++) {
		int x = rand() % (5*(n+1))-2;
		assert(d1.find(x) == d2.find(x));
	}
}


template<class T, class Dict, T (*gen)()>
void build_and_search(Dict &d, const char *name, int n) {
	srand(1);
	clock_t start = clock();
	for (int i = 0; i < n; i++)
		d.add(gen());
	clock_t stop = clock();
	double elapsed = ((double)(stop-start))/CLOCKS_PER_SEC;
	cout << name << " RANDOM ADD " << n << " " << elapsed << endl;

	long sum = 0;
	start = clock();
	for (int i = 0; i < 5*n; i++)
		sum += (int)d.find(gen());
	stop = clock();
	elapsed = ((double)(stop-start))/CLOCKS_PER_SEC;
	cout << name << " RANDOM FIND " << n << " " << elapsed << endl;
	cout << "sum = " << sum << endl;
}

template <class Dict>
void experiments(Dict &d, int n) {
	clock_t start = clock();
	for (int i = 0; i < n; i++)
		d.add(rand());
	clock_t stop = clock();
	double elapsed = ((double)(stop-start))/CLOCKS_PER_SEC;
	cout << "TDL" << " RANDOM ADD " << n << " " << elapsed << endl;

	int sum = 0;
	start = clock();
	for (int i = 0; i < 5*n; i++)
		sum += d.find(rand() % (5*n));
	stop = clock();
	elapsed = ((double)(stop-start))/CLOCKS_PER_SEC;
	cout << "TDL" << " RANDOM FIND " << n << " " << elapsed << endl;
	cout << "sum = " << sum << endl;

}

int main(int argc, char **argv) {

	DumbString<10> ds;
	cout << ds << endl;

	// Do some run-off comparisons just to make sure everything works
	{
		cout << "Testing TodoList versus Treap...";
		cout.flush();
		fastws::TodoList<int> tdl(NULL, 0, .41);
		ods::Treap1<int> t;
		test_dicts(tdl, t, 100000);
		cout << "done" << endl;
	}

	{
		cout << "Testing RedBlackTree versus Treap...";
		cout.flush();
		ods::RedBlackTree1<int> rbt;
		ods::Treap1<int> t;
		test_dicts(rbt, t, 100000);
		cout << "done" << endl;
	}

	{
		cout << "Testing Skiplist versus Treap...";
		cout.flush();
		ods::SkiplistSSet<int> sl;
		ods::Treap1<int> t;
		test_dicts(sl, t, 100000);
		cout << "done" << endl;
	}

	cout << "DumbStrings" << endl;
	int n = 500000;
	{
		ods::RedBlackTree1<DumbString<10> > sl;
		build_and_search<DumbString<10>,ods::RedBlackTree1<DumbString<10> >,gen_dumbstring<10> >(sl, "RedBlackTree", n);
	}
	{
		fastws::TodoList<DumbString<10> > tdl(NULL, 0, .2);
		build_and_search<DumbString<10>,fastws::TodoList<DumbString<10> >,gen_dumbstring<10> >(tdl, "TodoList", n);
	}
	{
		ods::SkiplistSSet<DumbString<10> > tdl;
		build_and_search<DumbString<10>,ods::SkiplistSSet<DumbString<10> >,gen_dumbstring<10> >(tdl, "Skiplist", n);
	}
	cout << "Done DumbStrings" << endl;
	for (int n = 1000000; n <= 10000000; n += 1000000) {
		{
			fastws::TodoList<int> tsl(NULL, 0, .41);
			build_and_search<int,fastws::TodoList<int>,rand>(tsl, "TodoList", n);
		}

		{
			ods::RedBlackTree1<int> sl;
			build_and_search<int,ods::RedBlackTree1<int>,rand>(sl, "RedBlackTree", n);
		}

		{
			ods::SkiplistSSet<int> sl;
			build_and_search<int,ods::SkiplistSSet<int>,rand>(sl, "Skiplist", n);
		}

		{
			ods::Treap1<int> t;
			build_and_search<int,ods::Treap1<int>,rand>(t, "Treap", n);
		}
	}

	return 0;
}

