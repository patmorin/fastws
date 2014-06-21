#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <ctime>
using namespace std;

#include "SkiplistSSet.h"
#include "Treap.h"
#include "SplayTree.h"
#include "wsskiplist.h"
#include "todolist.h"

// Compare the results of
template<class Dict1, class Dict2>
void test_dicts(Dict1 &d1, Dict2 &d2, int n) {
	for (int i = 0; i < n; i++) {
		int x = rand() % 5*n;
		assert(d1.add(x) == d2.add(x));
	}

	for (int i = 0; i < 5*n; i++) {
		int x = rand() % (5*(n+1))-2;
		assert(d1.find(x) == d2.find(x));
	}
}


template<class Dict>
void build_and_search(Dict &d, const char *name, int n) {
	clock_t start = clock();
	for (int i = 0; i < n; i++)
		d.add(rand() % (5*n));
	clock_t stop = clock();
	double elapsed = ((double)(stop-start))/CLOCKS_PER_SEC;
	cout << name << " RANDOM ADD " << n << " " << elapsed << endl;

	start = clock();
	for (int i = 0; i < 5*n; i++)
		d.find(rand() % (5*n));
	stop = clock();
	elapsed = ((double)(stop-start))/CLOCKS_PER_SEC;
	cout << name << " RANDOM FIND " << n << " " << elapsed << endl;
}

template<class Dict>
void random_searches(Dict &d, const char *name) {
	clock_t start = clock();
	int n = d.size();
	for (int i = 0; i < 3*n; i++) {
		d.find(rand() % n);
	}
	clock_t stop = clock();
	double elapsed = ((double)(stop-start))/CLOCKS_PER_SEC;
	cout << name << " RANDOM FIND " << n << " " << elapsed << endl;
}

template<class Dict>
void fractional_searches(Dict &d, char *name, double percentage) {
	int n = d.size();
	int m = n * percentage;
	int *queries = new int[m];
	for (int i = 0; i < m; i++)
		queries[i] = rand() % n;

	clock_t start = clock();
	for (int i = 0; i < 3*n; i++) {
		d.find(queries[rand() % m]);
	}
	clock_t stop = clock();
	delete[] queries;
	double elapsed = ((double)(stop-start))/CLOCKS_PER_SEC;
	cout << name << " FRACTIONAL(" << percentage << ") FIND "
			<< n << " " << elapsed << endl;
}


int main(int argc, char **argv) {

#ifdef XXXX
	int n = 43;

	cout << "Creating " << n << " data items" << endl;
	fastws::TodoList<int> tsl(NULL, 0, .45);
	for (int i = 0; i < n; i++) {
		tsl.add(48*i % (2*n));
		cout << tsl;
	}
	for (int i = 0; i < n; i++) {
		int x = rand() % (2*(n+1))-1;
		cout << x << " => " << tsl.find(x) << endl;
	}
	exit(0);
#endif // XXX

/*
	{
		for (double eps = .7; eps > .01; eps -= .01) {
			fastws::TodoList<int> tsl(NULL, 0, eps);
			char name[100];
			sprintf(name, "%f", eps);
			build_and_search(tsl, name, n);
		}
	}
*/

	for (int n = 10000; n <= 100000; n += 10000) {
		{
			fastws::TodoList<int> tsl(NULL, 0, .41);
			ods::Treap1<int> t;
			test_dicts(tsl, t, 1000000);
		}

		{
			fastws::TodoList<int> tsl(NULL, 0, .41);
			build_and_search(tsl, "TodoList", n);
		}

		{
			ods::SkiplistSSet<int> sl;
			build_and_search(sl, "Skiplist", n);
		}

		{
			ods::Treap1<int> t;
			build_and_search(t, "Treap", n);
		}
	}

#ifdef XXXXXX
	int *data = new int[n];
	for (int i = 0; i < n; i++)
		data[i] = i + 1;

	for (double percentage = 1; percentage > 0; percentage -= .01) {
		ods::SplayTree1<int> st;
		for (int i = 0; i < n; i++)
			st.add(data[i]);
		fractional_searches<ods::SplayTree1<int> >(st, "SplayTree", percentage);
	}

	for (double percentage = 1; percentage > 0; percentage -= .01) {
		ods::Treap1<int> sl;
		for (int i = 0; i < n; i++)
			sl.add(data[i]);
		fractional_searches<ods::Treap1<int> >(sl, "Treap", percentage);
	}

	for (double percentage = 1; percentage > 0; percentage -= .01) {
		ods::SkiplistSSet<int> sl;
		for (int i = 0; i < n; i++)
			sl.add(data[i]);
		fractional_searches<ods::SkiplistSSet<int> >(sl, "Skiplist", percentage);
	}

	double epsilon = .45;
	for (double percentage = 1; percentage > 0; percentage -= .01) {
		fastws::TodoList<int> sl(data, n, int_cmp, epsilon);
		fractional_searches<fastws::TodoList<int> >(sl, "TodoList", percentage);
	}
#endif // XXXXX

/*  Using the following code, we determined that the optimal for epsilon
 *  is about .45; but the running-times are _very_ sensitive to epsilon
	for (double epsilon = .6; epsilon >= .1; epsilon -= .01) {
		fastws::WSSkiplist<int> sl(data, n, int_cmp, epsilon);
		char name[100];
		sprintf(name, "WSSkiplist(%lf)", epsilon);
		random_searches<fastws::WSSkiplist<int> >(sl, name);
	}
*/



	// big_test(n, .44);
	return 0;
}

