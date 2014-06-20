#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <ctime>
using namespace std;

#include "SkiplistSSet.h"
#include "Treap.h"
#include "SplayTree.h"
#include "wsskiplist.h"
#include "topskiplist.h"


int int_cmp(const int &a, const int &b) {
	return a - b;
}

void big_test(int n, double eps) {
	cout << "Creating " << n << " data items" << endl;
	int *data = new int[n];
	for (int i = 0; i < n; i++)
		data[i] = i + 1;
	{
		cout << "Creating WSSkiplist" << endl;
		fastws::TopSkiplist<int> sl(data, n, int_cmp, eps);
		delete[] data;
		cout << "Doing " << 4*n << " searches" << endl;
		for (int i = 0; i < 4*n; i++) {
			sl.find(rand() % n);
		}
		cout << sl;
		cout << "Cleaning up WSSkiplist" << endl;
	}
	cout << "Done!" << endl;
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
	int n = 10000;
	cout << argc << endl;
	if (argc == 2)
		n = atoi(argv[1]);

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
		fastws::TopSkiplist<int> sl(data, n, int_cmp, epsilon);
		char name[100];
		sprintf(name, "WSSkiplist(%lf)", epsilon);
		fractional_searches<fastws::TopSkiplist<int> >(sl, name, percentage);
	}


/*  Using the following code, we determined that the optimal for epsilon
 *  is about .45; but the running-times are _very_ sensitive to epsilon
	for (double epsilon = .6; epsilon >= .1; epsilon -= .01) {
		fastws::WSSkiplist<int> sl(data, n, int_cmp, epsilon);
		char name[100];
		sprintf(name, "WSSkiplist(%lf)", epsilon);
		random_searches<fastws::WSSkiplist<int> >(sl, name);
	}
*/


/*
	int n = 40;

	cout << "Creating " << n << " data items" << endl;
	int *data = new int[n];
	for (int i = 0; i < n; i++) {
		data[i] = i + 1;
	}
	{
		cout << "Creating WSSkiplist" << endl;
		fastws::WSSkiplist<int> sl(data, n);
		delete[] data;
		cout << sl;
		sl.find(16);
		cout << sl;
		sl.find(22);
		cout << sl;
		cout << "Cleaning up WSSkiplist" << endl;
		for (int i = 0; i < n; i++) {
			sl.find(rand() % n);
			cout << sl;
		}
	}
	cout << "Done!" << endl;
*/

	// big_test(n, .44);
	return 0;
}

