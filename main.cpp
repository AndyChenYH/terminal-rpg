#include <bits/stdc++.h>
#include <ncurses.h>
#include <unistd.h>
using namespace std;

// output file for debugging
ofstream fout("out.txt");

namespace __hidden__ { struct print { bool space; print() : space(false) {} ~print() { fout << endl; } template <typename T> print &operator , (const T &t) { if (space) fout << ' '; else space = true; fout << t; return *this; } }; }
#define print __hidden__::print(),

class Block {
	public:
	bool pass;
	char look;
	Block() {
		pass = true;
		look = '.';
	}
	Block(bool pass, char look) : pass(pass), look(look) {
	}
};
// initializing map first so portal can use it
class Map {
	public:
	int row, col;
	// map data
	vector<vector<Block>> data;
	// portals translates current map's coordinates to another map's coordinates
	map<pair<int, int>, tuple<Map, int, int>> ports;
	Map() {}
	Map(int row, int col) : row(row), col(col) {
		data = vector<vector<Block>>(row, vector<Block>(col));
	}
	bool inBound(int i, int j) {
		return 0 <= i && i < row && 0 <= j && j < col;
	}
};

Map world(30, 30);
Map inn(10, 10);
// current map
Map curMap;
class Player {
	public:
	int i, j;
	Player(int i, int j) : i(i), j(j) {}	
	// directional vectors
	void move(int di, int dj) {
		i += di, j += dj;
		print i, j;
		// seeing if block moved on is a portal
		auto fid = curMap.ports.find({i, j});
		if (fid != curMap.ports.end()) {
			// changing current map and player coordinates
			// FIXME
//			tuple<Map, int, int> tp = fid->second;
			curMap = get<0>(fid->second);
			int a = get<1>(fid->second);
			int b = get<2>(fid->second);
			fout << a;
//			i = get<1>(fid->second);
//			j = get<2>(fid->second);
		}
	}
};
// should be even numbers
const int camHei = 20, camWid = 20;
Player player(5, 5);

int main() {
	initscr();
	cbreak();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	noecho();
	// setting up the world
	world.data[10][10].look = '(';
	inn.data[5][5].look = '%';
	world.ports.insert({{1, 1}, {inn, 3, 3}});
	// since curMap is a new copy, it has to be assigned to the world after initialization
	curMap = world;
	int frame = 0;
	while (true) {
		// clears screen of any output before next cycle
		clear();
		// drawing the scene within camera scope
		for (int i = 0; i < camWid; i ++) {
			for (int j = 0; j < camHei; j ++) {
				int ci = player.i + i - camHei / 2;
				int cj = player.j + j - camWid / 2;
				// draw player
				if (ci == player.i && cj == player.j) {
					mvaddch(i, j, '@');
					assert(curMap.inBound(ci, cj));
				}
				// draw world tile
				else if (curMap.inBound(ci, cj)) {
					mvaddch(i, j, curMap.data[ci][cj].look);
				}
				// empty void
				else {
					mvaddch(i, j, ' ');
				}
			}
		}
		refresh();
		char inp = getch();
		if (inp == 'w') player.move(-1, 0);
		else if (inp == 's') player.move(1, 0);
		else if (inp == 'a') player.move(0, -1);
		else if (inp == 'd') player.move(0, 1);

		if (frame == 5000000) frame = 0;
	}
	endwin();
}

