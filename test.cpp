 
#include <bits/stdc++.h>
using namespace std;

/*
class Hi {
	int a;
	Hi(int a) : a(a) {}
};
*/
int main() {
	map<int, tuple<int, int, int>> mp = {
		{1, {2, 3, 4}}
	};
	auto fid = mp.find(1);
	tuple<int, int, int> tp = fid->second;
	int i = get<0>(fid->second);
	printf("%d %d %d\n", get<0>(tp), get<1>(tp), get<2>(tp));
}
