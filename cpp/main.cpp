#include <iostream>
#include <cstdlib>
using namespace std;

#include "wsskiplist.h"

void big_test(int n) {
	cout << "Creating " << n << " data items" << endl;
	int *data = new int[n];
	for (int i = 0; i < n; i++)
		data[i] = i + 1;
	{
		cout << "Creating WSSkiplist" << endl;
		fastws::WSSkiplist<int> sl(data, n);
		delete[] data;
		sl.find(16);
		sl.find(22);
		cout << "Doing lots of searches" << endl;
		for (int i = 0; i < n; i++) {
			sl.find(rand() % n);
		}
		cout << "Cleaning up WSSkiplist" << endl;
	}
	cout << "Done!" << endl;
}

int main(int argc, char **argv) {
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

	big_test(50000);
	return 0;
}

