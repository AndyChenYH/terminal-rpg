#include <bits/stdc++.h>
using namespace std;

int main() {
	ifstream fin("assets/manface1.txt");
	string ln;
	while (getline(fin, ln)) {
		cout << ln << endl;
		printf("%d\n", (int) ln.size());
	}
}
