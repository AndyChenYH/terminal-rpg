#include <bits/stdc++.h>
#include <ncurses.h>
#include <unistd.h>
using namespace std;

// output file for debugging
ofstream fout("out.txt");

// python2 style print function for debugging; outputs with fout
namespace __hidden__ { struct print { bool space; print() : space(false) {} ~print() { fout << endl; } template <typename T> print &operator , (const T &t) { if (space) fout << ' '; else space = true; fout << t; return *this; } }; }
#define print __hidden__::print(),

const vector<pair<int, int>> drs = {{-1, 0}, {0, -1}, {1, 0}, {0, 1}};
int main() {
	int row = 0, col = 0;
	vector<vector<bool>> grid;
	ifstream fin("in.txt");
	string ln;
	while (getline(fin, ln)) {
		col = ln.size();
		grid.push_back(vector<bool>(col));
		for (int j = 0; j < col; j ++) {
			grid[row][j] = ln[j] - '0';
		}
		row ++;
	}
	initscr();
	cbreak();
	noecho();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	int pI = 0, pJ = 0;
	set<pair<int, int>> enemies;
	for (int cc = 0; cc < 10; ) {
		int i = rand() % row, j = rand() % col;
		if (grid[i][j] == 0 && enemies.find({i, j}) == enemies.end()) {
			enemies.insert({i, j});
			cc ++;
		}
	}
	auto canGo = [&] (int i, int j) -> bool {
		return 0 <= i && i < row && 0 <= j && j < col && grid[i][j] == 0 && enemies.find({i, j}) == enemies.end();
	};
	int frame = 0;
	while (true) {
		assert(enemies.size() == 10);
		usleep(20000);
		frame ++;
		erase();
		for (int i = 0; i < row; i ++) {
			for (int j = 0; j < col; j ++) {
				mvaddch(i, j, grid[i][j] ? '1' : ' ');
			}
		}
		for (pair<int, int> p : enemies) {
			mvaddch(p.first, p.second, '%');
		}
		mvaddch(pI, pJ, '@');
		mvaddstr(20, 60, "hi");
		refresh();
		int ch = getch();
		if (ch == 'a' && canGo(pI, pJ - 1)) pJ --;
		else if (ch == 'd' && canGo(pI, pJ + 1)) pJ ++;
		else if (ch == 'w' && canGo(pI - 1, pJ)) pI --;
		else if (ch == 's' && canGo(pI + 1, pJ)) pI ++;
		if (frame % 10 == 0) {
			int sI = pI, sJ = pJ;
			vector<vector<bool>> vis(row, vector<bool>(col, false));
			vector<vector<pair<int, int>>> parent(row, vector<pair<int, int>>(col, {-1, -1}));
			list<pair<int, int>> q;
			vis[sI][sJ] = 0;
			q.push_back({sI, sJ});
			while (!q.empty()) {
				int i = q.front().first, j = q.front().second;
				q.pop_front();
				for (pair<int, int> d : drs) {
					int ni = i + d.first, nj = j + d.second;
					if (0 <= ni && ni < row && 0 <= nj && nj < col && grid[ni][nj] == 0) {
						if (!vis[ni][nj]) {
							q.push_back({ni, nj});
							vis[ni][nj] = true;
							parent[ni][nj] = {i, j};
						}
					}
				}
			}
			set<pair<int, int>> tmp;
			for (pair<int, int> p : enemies) {
				// randomly selected as yes, and also is more than 1 block away from player
				if (rand() % 2 == 0 && abs(pI - p.first) + abs(pJ - p.second) > 1) {
					vector<pair<int, int>> randomDrs(drs.begin(), drs.end());
					shuffle(randomDrs.begin(), randomDrs.end(), default_random_engine(time(0)));
					for (pair<int, int> dr : randomDrs) {
						int ni = p.first + dr.first, nj = p.second + dr.second;
						if (canGo(ni, nj) && tmp.find({ni, nj}) == tmp.end() && make_pair(ni, nj) != make_pair(pI, pJ)) {
							tmp.insert({ni, nj});
							goto L1;
						}
					}
				}
				// nowhere to go or didn't get chosen to random move, so don't move it
				tmp.insert(p);
L1:;
			}
			enemies = tmp;

			set<pair<int, int>> newPos;
			for (pair<int, int> p : enemies) {
				pair<int, int> np = parent[p.first][p.second];
				if (np == make_pair(-1, -1)) {
					print p.first, p.second;
					usleep(999999999);
					return 0;
				}
				if (newPos.find(np) != newPos.end() || enemies.find(np) != enemies.end() 
						|| np == make_pair(sI, sJ)) newPos.insert(p);
				else newPos.insert(np);
			}
			enemies = newPos;
		}
	}
}
