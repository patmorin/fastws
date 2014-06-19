#include <iostream>
using namespace std;

#include "wsskiplist.h"


int main(int argc, char **argv) {
	int n = 100;

	cout << "Creating " << n << " data items" << endl;
	int *data = new int[n];
	for (int i = 0; i < n; i++) {
		data[i] = i+1;
	}
	{
		cout << "Creating WSSkiplist" << endl;
		fastws::WSSkiplist<int> sl(data, n);
		cout << sl;
		sl.find(16);
		cout << sl;
		cout << "Cleaning up WSSkiplist" << endl;
	}
	cout << "Done!" << endl;
	return 0;
}
