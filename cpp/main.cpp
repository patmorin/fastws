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


// A silly class to use for simulating classes that have more expensive
// comparisons
class Integer {
protected:
	// used for statistics gathering
	static unsigned long dummy;
	static size_t comparisons;
	static size_t del;

	// the actual integer
	int data;

	void delay() {
		unsigned int tmp = 0;
		for (size_t i = 0; i < del; i++) {
			tmp = (tmp + data) % 733721;
			dummy += tmp;  // just to ensure this doesn't get optimized away
		}
		comparisons++;
	}
public:
	static void setDelay(size_t delay) {
		del = delay;
	}
	static size_t getComparisons() {
		return comparisons;
	}
	static void resetComparisons() {
		comparisons = 0;
	}
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

size_t Integer::dummy, Integer::comparisons, Integer::del;


ostream& operator<<(ostream &out, Integer &ds) {
	ds.printOn(out);
	return out;
}

static long summer;

int rand_data(size_t i, size_t n) {
	return (rand() % (n*5));
}

int rand_search(size_t i, size_t n) {
	return rand() % (n*5) - 2;
}

int sequential_data(size_t i, size_t n) {
	return 5*i;
}

int requential_data(size_t i, size_t n) {
	return 5*(n-i-1);
}

int shuffle_data(size_t i, size_t n) {
	int sn = ceil(sqrt(n));
	return (i*sn + i/sn) % n;
}

template<class Dict>
void build_and_search(Dict &d, const char *name, size_t n,
		int (*gen_add)(size_t, size_t), int (*gen_search)(size_t, size_t)) {
	srand(1);
	Integer::resetComparisons();

	clock_t start = clock();
	for (size_t i = 0; i < n; i++)
		d.add(gen_add(i, n));
	clock_t stop = clock();

	double elapsed = ((double)(stop-start))/CLOCKS_PER_SEC;
	double avg = ((double)Integer::getComparisons()) / n;
	double c = avg * log(2) / log(d.size());
	cout << name << " ADD " << n << " " << elapsed
			<< " " << Integer::getComparisons()
			<< " " << c << endl;

	Integer::resetComparisons();
	long sum = 0;
	start = clock();
	for (size_t i = 0; i < 5*n; i++)
		sum += (int)d.find(gen_search(i, n));
	stop = clock();

	elapsed = ((double)(stop-start))/CLOCKS_PER_SEC;
	avg = ((double)Integer::getComparisons()) / (5*n);
	c = avg * log(2) / log(d.size());

	cout << name << " FIND " << d.size() << " " << elapsed
			<< " " <<  Integer::getComparisons()
			<< " " << c << endl;

	summer += sum; // to make sure this isn't optimized away
}


void test_suite(size_t n, int (*gen_data)(size_t, size_t),
		int (*gen_search)(size_t, size_t)) {
	cout << "Structure Operation n time #comparisons c" << endl;
	{
		fastws::TodoList<Integer> tdl(NULL, 0, .2);
		build_and_search(tdl, "TodoList", n, gen_data, gen_search);
	}
	{
		ods::RedBlackTree1<Integer> rbt;
		build_and_search(rbt, "RedBlackTree", n, gen_data, gen_search);
		int tpl = rbt.tpl();
		cout << "I: total path length = " << tpl << endl;
		double apl = ((double)tpl) / rbt.size();
		cout << "I: average path length = " << apl << endl;
		cout << "I: c_average = " << apl * log(2) / log(n) << endl;
		cout << "#red nodes = " << rbt.reds() << endl;
		int mpl = rbt.mpl();
		cout << "I: maximum path length = " << mpl << endl;
		cout << "I: c_max = " << log(2) * mpl / log(n) << endl;
	}
	{
		ods::Treap1<Integer> t;
		build_and_search(t, "Treap", n,	gen_data, gen_search);
		int tpl = t.tpl();
		cout << "I: total path length = " << tpl << endl;
		double apl = ((double)tpl) / t.size();
		cout << "I: average path length = " << apl << endl;
		cout << "I: c_average = " << apl * log(2) / log(n) << endl;
		int mpl = t.mpl();
		cout << "I: maximum path length = " << mpl << endl;
		cout << "I: c_max = " << log(2) * mpl / log(n) << endl;
	}
	{
		ods::SkiplistSSet<Integer> sl;
		build_and_search(sl, "Skiplist", n, gen_data, gen_search);
	}

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


void sanity_tests(size_t n) {
	{
		ods::RedBlackTree1<int> rbt;
		ods::Treap1<int> t;
		test_dicts(rbt, t, n);
	}
	{
		ods::Treap1<int> t;
		ods::SkiplistSSet<int> sl;
		test_dicts(t, sl, n);
	}
	{
		ods::SkiplistSSet<int> sl;
		fastws::TodoList<int> tdl;
		test_dicts(sl, tdl, n);
	}
	{
		fastws::TodoList<int> tdl;
		ods::RedBlackTree1<int> rbt;
		test_dicts(tdl, rbt, n);
	}
}

int main(int argc, char **argv) {
	// start with some sanity tests
	cout << "I: Doing sanity tests...";
	cout.flush();
	sanity_tests(100000);
	cout << "done" << endl;

	// start with some bigger tests
	for (size_t delay = 0; delay <= 0; delay += 10) {
		Integer::setDelay(delay);
		cout << "DELAY " << delay << endl;
		size_t n = 500000;
		cout << endl << "Random additions" << endl;
		test_suite(n, rand_data, rand_search);
		cout << endl << "Sequential additions" << endl;
		test_suite(n, sequential_data, rand_search);
		cout << endl << "Requential additions" << endl;
		test_suite(n, requential_data, rand_search);
		cout << endl;
		cout << endl << "Shuffled additions" << endl;
		test_suite(n, shuffle_data, rand_search);
		cout << endl;
	}
}











/*

Integer generate_random() {
	return Integer(rand());
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

template<class T, class Dict>
void build_and_search(Dict &d, const char *name, int n,
		T (*gen_add)(), T(*gen_search)()) {
	global_comparisons = 0;

	srand(1);
	clock_t start = clock();
	for (int i = 0; i < n; i++)
		d.add(gen_add());
	clock_t stop = clock();
	double elapsed = ((double)(stop-start))/CLOCKS_PER_SEC;
	cout << name << " RANDOM ADD " << n << " " << elapsed << endl;

	cout << "comparisons = " << global_comparisons << endl;
	global_comparisons = 0;

	long sum = 0;
	start = clock();
	for (int i = 0; i < 5*n; i++)
		sum += (int)d.find(gen_search());
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
		build_and_search(sl, "RedBlackTree", n, generate_random<Integer<10>,
				generate_random<Integer<10>);
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

*/
