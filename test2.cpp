#include <bits/stdc++.h>
using namespace std;

class Hi {
	public:
	pair<Hi, int> pr;
	int d;
	Hi() {}
	Hi(int d) : d(d) {}
};

int main() {
	/*
	Hi a(10);
	a.tp = {Hi(20), 6};
	printf("%d\n", get<0>(a.tp));
	*/
}
/*
class Hi {
	public:
	map<int, tuple<Hi, int>> mp;
	int d;
	Hi() {}
	Hi(int d) : d(d) {}
};

int main() {
	Hi a(10);
	a.mp.insert({5, {Hi(20), 6}});
	auto fid = a.mp.find(5);
	a = get<0>(fid->second);
	printf("%d\n", get<1>(fid->second));
}
*/
