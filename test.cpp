#include <bits/stdc++.h>
using namespace std;

class Lol {
	public:
	int d;
	Lol(int d) : d(d) {}
};
void hi(Lol *lo) {
	lo->d = 20;
}
int main() {
	Lol lol(10);
	hi(&lol);
	printf("%d\n", lol.d);
}
