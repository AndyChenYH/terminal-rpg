#include <bits/stdc++.h>
using namespace std;

int main() {
	vector<int> v = {1, 2, 3, 4, 5};
	*find(v.begin(), v.end(), 2) = 10;
	printf("%d\n", v[1]);
}
