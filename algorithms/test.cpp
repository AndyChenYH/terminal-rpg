#include <bits/stdc++.h>
using namespace std;

int main() {
	vector<int> a = {1, 2, 3, 4};
	shuffle(a.begin(), a.end(), default_random_engine(time(0)));
	for (int d : a) printf("%d ", d);
	puts("");
}
