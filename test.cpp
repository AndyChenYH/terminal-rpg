#include <stdio.h>
#include <functional>
using namespace std;

function<int(int)> lam;

void func() {
	int a = 5;
	lam = [=] (int b) -> int {
		return a + b;
	};
}

int main() {
	func();
	printf("%d\n", lam(6));

}
