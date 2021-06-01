#include <bits/stdc++.h>
#include <ncurses.h>
#include <unistd.h>
using namespace std;

// output file for debugging
ofstream fout("out.txt");

namespace __hidden__ { struct print { bool space; print() : space(false) {} ~print() { fout << endl; } template <typename T> print &operator , (const T &t) { if (space) fout << ' '; else space = true; fout << t; return *this; } }; }
#define print __hidden__::print(),

// max frame
const int MFrame = 1e9;
int frame = 0;
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
class Item {
	public:
	// weapon is "w-name", resource is "r-name"
	string name;
	int amount, damage;
	char look;
	int regrowTime;
	vector<vector<bool>> aoe;
	// resource initialization
	Item(string name, char look) : name(name), amount(1), look(look), regrowTime(0) { }
};
class NPC {
	public:
	string name;
	int diaNum;
	vector<string> dialogue;
	NPC(string name, vector<string> dialogue) : name(name), diaNum(0), dialogue(dialogue) { }

};
// initializing map first so portal can use it
class Map {
	public:
	int row, col;
	// map data
	vector<vector<Block>> data;
	// portals translates current map's coordinates to another map's coordinates
	map<pair<int, int>, tuple<Map*, int, int>> ports;
	// coordinates of resource nodes
	map<pair<int, int>, Item> resources;
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
Map *curMap = &world;
// should be even numbers
const int camHei = 20, camWid = 20;
class Player {
	public:
	int i, j, faceI, faceJ;
	int health;
	vector<Item> inventory;
	Player(int i, int j) : i(i), j(j), health(10) {}	
	// directional vectors
	void move(int di, int dj) {
		faceI = di, faceJ = dj;
		i += di, j += dj;
		// seeing if block moved on is a portal
		auto fid = curMap->ports.find({i, j});
		if (fid != curMap->ports.end()) {
			curMap = get<0>(fid->second);
			i = get<1>(fid->second);
			j = get<2>(fid->second);
		}
	}
	void act() {
		// coordinates of block the player is targeting
		int ci = i + faceI, cj = j + faceJ;

		// see whether the target block has a resource
		auto fid = curMap->resources.find({ci, cj});
		if (fid != curMap->resources.end() && fid->second.regrowTime < frame) {
			// does the player's inventory already have at least one of the resource?
			// if so, just increment the amount
			bool has = false;
			for (int ii = 0; ii < (int) inventory.size(); ii ++) {
				if (inventory[ii].name == fid->second.name) {
					inventory[ii].amount ++;
					has = true;
					break;
				}
			}
			// if the resource isn't already in the inventory, make a new spot for it
			if (!has) inventory.push_back(fid->second);
			// wait 100000 frames until it regrows
			fid->second.regrowTime = (frame + 100000) % MFrame;
		}
	}
	void dispInventory() {
		// display the items in the inventory from top to bottom
		for (int ii = 0; ii < (int) inventory.size(); ii ++) {
			mvaddstr(ii, camWid + 2, (inventory[ii].name + " x " + to_string(inventory[ii].amount)).c_str());
		}
	}
};
Player player(5, 5);

int main() {
	initscr();
	cbreak();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	noecho();
	// testing with random characters
	world.data[10][10].look = '(';
	inn.data[5][5].look = '%';
	// portal from world to inn
	world.ports.insert({{1, 1}, {&inn, 3, 3}});
	inn.ports.insert({{9, 9}, {&world, 0, 0}});
	// testing with rose resource node
	world.resources.insert({{6, 6}, Item("r-rose", '&')});
	world.resources.insert({{8, 3}, Item("r-honey", '+')});
	while (true) {
		// clears screen of any output before next cycle
		clear();
		// drawing the scene within camera scope
		// i and j are the cli screen positions
		for (int i = 0; i < camWid; i ++) {
			for (int j = 0; j < camHei; j ++) {
				// ci and cj are the map relative positives
				int ci = player.i + i - camHei / 2;
				int cj = player.j + j - camWid / 2;
				// draw player
				if (ci == player.i && cj == player.j) {
					mvaddch(i, j, '@');
					// make sure the player doesn't go out of bounds
					assert(curMap->inBound(ci, cj));
				}
				// draw world tile
				else if (curMap->inBound(ci, cj)) {
					// see if a port or resource node has to be drawn
					auto fPort = curMap->ports.find({ci, cj});
					auto fResource = curMap->resources.find({ci, cj});
					if (fPort != curMap->ports.end()) {
						// make sure ports and resource node aren't on the same block
						assert(fResource == curMap->resources.end());
						mvaddch(i, j, '^');
					}
					// draw resource node if it exists in this block
					else if (fResource != curMap->resources.end()) {
						// if the resource node has regrown
						print ">", frame;
						if (fResource->second.regrowTime < frame) mvaddch(i, j, fResource->second.look);
					}
					// draw normal map block
					else mvaddch(i, j, curMap->data[ci][cj].look);
				}
				// empty void for things that go out of the map
				else {
					mvaddch(i, j, ' ');
				}
			}
		}
		player.dispInventory();
		// uploads drawing onto terminal
		refresh();
		char inp = getch();
		if (inp == 'w') player.move(-1, 0);
		else if (inp == 's') player.move(1, 0);
		else if (inp == 'a') player.move(0, -1);
		else if (inp == 'd') player.move(0, 1);
		// action
		else if (inp == ' ') player.act();

		frame ++;
		if (frame >= MFrame) frame = 0;
	}
	endwin();
}

