#include <bits/stdc++.h>
using namespace std;

int main() {
	vector<function<void(int*)>> v;
	function<void(int*)> f1 = [&] (int *d) {
		*d = *d + 1;
	};
	v.push_back(f1);
	int a = 2;
	v[0](&a);
	v[0](&a);
	printf("%d\n", a);
}
