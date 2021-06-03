#include <bits/stdc++.h>
using namespace std;

// test modifiability of class object

class Lol {
	public:
	int a;
	Lol() {}
	Lol(int a) : a(a) {}
};
pair<int, Lol> pp = {76, Lol(3)};
class Hi {
	public:
	int d;
	map<int, Lol> mp;
	Hi() {}
	Hi(int d) {
		cin >> d;
		mp.insert({5, pp.second});
	}
};
Hi h(10);
Hi *c = &h;
int main() {
	c->mp[5].a = 34;
	printf("%d\n", pp.second.a);
}
