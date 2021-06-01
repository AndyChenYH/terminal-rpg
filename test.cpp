#include <bits/stdc++.h>
using namespace std;


class Map {
	public:
	int row, col;
	map<pair<int, int>, tuple<Map, int, int>> ports;
	Map() {}
	Map(int row, int col) : row(row), col(col) {}
};

Map world(30, 30);
Map inn(10, 10);
// current map
Map curMap;


const int camHei = 20, camWid = 20;

int main() {
	world.ports.insert({{1, 1}, {inn, 3, 3}});
	curMap = world;
	auto fid = curMap.ports.find({1, 1});
	curMap = get<0>(fid->second);
	int i = get<1>(fid->second);
	int j = get<2>(fid->second);
	printf("%d %d\n", i, j);
}
