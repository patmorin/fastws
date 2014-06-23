#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <string>
#include <algorithm>
#include <iterator>
using namespace std;

#include <unistd.h>

#include "SkiplistSSet.h"
#include "Treap.h"
#include "SplayTree.h"
#include "RedBlackTree.h"
#include "wsskiplist.h"
#include "todolist.h"
#include "todolist2.h"


static long global_sum = 0;
static long global_comparisons = 0;

// A silly class to use for testing more expensive comparisons
template<size_t del>
class Integer {
protected:

	int data;

	void delay() {
		int tmp = 0;
		for (int i = 0; i < del; i++) {
			tmp = (tmp + data) % 733721;
			global_sum += tmp;
		}
		global_comparisons++;
	}
public:
	Integer() {
		data = 0;
	}
	Integer(int i) {
		data = i;
	}
	Integer(const Integer &i) {
		data = i.data;
	}
	bool operator <(const Integer &other) {
		delay();
		return data < other.data;
	}
	bool operator >(const Integer &other) {
		// not delaying here means binary search trees are only charged once
		// for a three way comparison
		return data > other.data;
	}
	bool operator ==(const Integer &other) {
		return data == other.data;
	}
	void printOn(ostream &out) {
		out << data;
	}
	operator int() const {
		return data;
	}

};


template<size_t del>
ostream& operator<<(ostream &out, Integer<del> &ds) {
	ds.printOn(out);
	return out;
}

template<size_t del>
Integer<del> generate_random() {
	return Integer<del>(rand());
}

static int g_start = 0;
template<size_t del>
Integer<del> generate_sequential() {
	return Integer<del>(g_start++);
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
	global_comparisons = 0;

	srand(1);
	clock_t start = clock();
	for (int i = 0; i < n; i++)
		d.add(gen());
	clock_t stop = clock();
	double elapsed = ((double)(stop-start))/CLOCKS_PER_SEC;
	cout << name << " RANDOM ADD " << n << " " << elapsed << endl;

	cout << "comparisons = " << global_comparisons << endl;
	global_comparisons = 0;

	long sum = 0;
	start = clock();
	for (int i = 0; i < 5*n; i++)
		sum += (int)d.find(gen());
	stop = clock();
	elapsed = ((double)(stop-start))/CLOCKS_PER_SEC;
	cout << name << " RANDOM FIND " << n << " " << elapsed << endl;

	cout << "sum = " << sum << endl;
	cout << "comparisons = " << global_comparisons << endl;
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

	Integer<10> ds;
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
	int n = 100000;
	{
		ods::RedBlackTree1<Integer<10> > sl;
		build_and_search<Integer<10>,ods::RedBlackTree1<Integer<10> >,generate_random<10> >(sl, "RedBlackTree", n);
	}
	{
		fastws::TodoList<Integer<10> > tdl(NULL, 0, .1);
		build_and_search<Integer<10>,fastws::TodoList<Integer<10> >,generate_random<10> >(tdl, "TodoList", n);
	}
	{
		ods::SkiplistSSet<Integer<10> > sl;
		build_and_search<Integer<10>,ods::SkiplistSSet<Integer<10> >,generate_random<10> >(sl, "Skiplist", n);
	}
	{
		ods::Treap1<Integer<10> > t;
		build_and_search<Integer<10>,ods::Treap1<Integer<10> >,generate_random<10> >(t, "Treap", n);
	}
	cout << "Done DumbStrings" << endl;
	cout << generate_sequential<10>() << endl;
	cout << generate_sequential<10>() << endl;
	cout << generate_sequential<10>() << endl;
	{
		ods::RedBlackTree1<Integer<10> > sl;
		build_and_search<Integer<10>,ods::RedBlackTree1<Integer<10> >,generate_sequential<10> >(sl, "RedBlackTree", n);
	}
	{
		fastws::TodoList<Integer<10> > tdl(NULL, 0, .1);
		build_and_search<Integer<10>,fastws::TodoList<Integer<10> >,generate_sequential<10> >(tdl, "TodoList", n);
	}

	return 0;
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

