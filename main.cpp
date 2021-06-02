#include <bits/stdc++.h>
#include <ncurses.h>
#include <unistd.h>
using namespace std;

// output file for debugging
ofstream fout("out.txt");

// python2 style print function for debugging; outputs with fout
namespace __hidden__ { struct print { bool space; print() : space(false) {} ~print() { fout << endl; } template <typename T> print &operator , (const T &t) { if (space) fout << ' '; else space = true; fout << t; return *this; } }; }
#define print __hidden__::print(),

// should take more than a year to overflow
int frame = 0;
bool isTalking = false;
bool viewInventory = false;
// forward declarations
class Player;

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
// single dialogue/text box popup
class Dialogue {
	public:
	string words;
	bool hasTrigger;
	// when this dialogue is passed, modifies the player such as giving them a quest or items
	// return true if dialogue is allowed to advance (eg: player has met a certain condition)
	function<bool(Player*)> trigger;
	// by default, there's no trigger, since most dialogues are simply text/storytelling
	Dialogue(string words) : words(words), hasTrigger(false) { }
	Dialogue(string words, function<bool(Player*)> trigger) 
		: words(words), hasTrigger(true), trigger(trigger) {}
};
class NPC {
	public:
	string name;
	int diaNum;
	// linear dialogue consisting of words, and a function that runs when player presses enter
	// can do things such as give player items or quests
	vector<Dialogue> dialogues;
	NPC(string name, vector<Dialogue> dialogues) : 
		name(name), diaNum(0), dialogues(dialogues) { }
};
// initializing map first so portal can use it
class Map {
	public:
	string description;
	int row, col;
	// map data
	vector<vector<Block>> data;
	// portals translates current map's coordinates to another map's coordinates
	map<pair<int, int>, tuple<Map*, int, int>> ports;
	// coordinates of resource nodes
	map<pair<int, int>, Item> resources;
	// coordinates of NPCs
	map<pair<int, int>, NPC> npcs;
	Map(string description, int row, int col) : description(description), row(row), col(col) {
		data = vector<vector<Block>>(row, vector<Block>(col));
	}
	bool inBound(int i, int j) {
		return 0 <= i && i < row && 0 <= j && j < col;
	}
};

Map world("world map", 30, 30);
Map inn("cozy inn", 10, 10);
// current map
Map *curMap = &world;
NPC *curNPC;
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
		int newI = i + di, newJ = j + dj;
		if (!curMap->inBound(newI, newJ) || !curMap->data[newI][newJ].pass) return;
		i = newI, j = newJ;
		// seeing if block moved on is a portal
		auto fid = curMap->ports.find({i, j});
		if (fid != curMap->ports.end()) {
			curMap = get<0>(fid->second);
			i = get<1>(fid->second);
			j = get<2>(fid->second);
		}
	}
	void addItem(Item newIt) {
		// does the player's inventory already have at least one of the resource?
		// if so, just increment the amount
		for (Item &it : inventory) {
			if (it.name == newIt.name) {
				it.amount += newIt.amount;
				return;
			}
		}
		// if the item isn't already in the inventory, make a new spot for it
		inventory.push_back(newIt);
	}
	// if item exists in inventory, take it out and return true
	// otherwise return false
	bool takeItem(string takeIt) {
		for (int ii = 0; ii < (int) inventory.size(); ii ++) {
			if (takeIt== inventory[ii].name) {
				inventory[ii].amount --;
				if (inventory[ii].amount == 0) inventory.erase(inventory.begin() + ii);
				return true;
			}
		}
		return false;
	}
	void act() {
		// coordinates of block the player is targeting
		int ci = i + faceI, cj = j + faceJ;

		// see whether the target block has a resource
		auto fResource = curMap->resources.find({ci, cj});
		if (fResource != curMap->resources.end() && fResource->second.regrowTime < frame) {
			addItem(fResource->second);
			// wait 100000 frames until it regrows
			fResource->second.regrowTime = frame + 200;
		}
		// see if player has chosen to interact with an npc
		auto fNPC = curMap->npcs.find({ci, cj});
		if (fNPC != curMap->npcs.end()) {
			isTalking = true;
			curNPC = &(fNPC->second);
		}
	}
	// relative coordinates to allow easy translation of object
	void dispInventory(int relI, int relJ) {
		/* 
		 * -------------------------------
		 * |&   50 |$    10|=   40|		|
		 * |-------|-------|------|------|
		 * |+   120|       |      |      |
		 * |-------|-------|------|------|
		 * |%   300|       |      |      |
		 * |-------|-------|------|------|
		 * |       |       |      |      |
		 * |-------|-------|------|------|
		 * |       |       |      |      |
		 * --------|-------|------|------|
		 *
		 */
		// current index in player inventory

		int ci = 0;
		// draw first top bar
		mvaddstr(relI, relJ, "---------------------------------");
		// draw 5 rows of boxes
		for (int ii = 0; ii < 5; ii ++) {
			mvaddstr(1 + relI + ii * 2, relJ, "|       |       |       |       |");
			mvaddstr(2 + relI + ii * 2, relJ, "|-------|-------|-------|-------|");
		}
		// the location in the inventory matrix
		for (int ii = 0; ci < (int) inventory.size(); ii ++) {
			for (int jj = 0; ci < (int) inventory.size() && jj < 4; jj ++) {
				// draw the character look of the item and amount
				mvaddstr(1 + relI + ii * 2, 1 + relJ + jj * 8, 
						(string(1, inventory[ci].look) + "  " + to_string(inventory[ci].amount)).c_str()); 
				ci ++;
			}
		}
	}
};

Player player(5, 5);
int main() {
	initscr();
	cbreak();
	// allows getch() to get input at any time, without waiting for input
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	noecho();
	
	// testing with random characters
	world.data[10][10].look = '(';
	world.data[10][10].pass = false;
	world.data[11][10].look = '(';
	world.data[11][10].pass = false;
	inn.data[5][5].look = '%';
	// portal from world to inn
	world.ports.insert({{1, 1}, {&inn, 3, 3}});
	// portal from inn to world
	inn.ports.insert({{9, 9}, {&world, 0, 0}});
	// testing with rose resource node
	world.resources.insert({{6, 6}, Item("r-rose", '&')});
	world.resources.insert({{8, 3}, Item("r-honey", '+')});
	world.resources.insert({{6, 5}, Item("r-cactus", '}')});
	world.resources.insert({{6, 3}, Item("r-apple", 'q')});

	// test function
	function<bool(Player*)> func = [&] (Player *p) -> bool {
		bool res = p->takeItem("r-rose");
		if (res) p->addItem(Item("r-gold", 'G'));
		return res;
	};
	NPC npc1("Joe", {Dialogue("give me 1 rose", func), Dialogue("thank you!")});
	world.npcs.insert({{3, 4}, npc1});
	while (true) {
		// 50 refreshes a second
		usleep(20000);
		frame ++;
		// make sure frame doesn't overflow
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
				// drawing the facing direction
				else if (ci == player.i + player.faceI && cj == player.j + player.faceJ) {
					mvaddch(i, j, 'o');
				}
				// draw world tile
				else if (curMap->inBound(ci, cj)) {
					// see if a port or resource node or npc has to be drawn
					auto fPort = curMap->ports.find({ci, cj});
					auto fResource = curMap->resources.find({ci, cj});
					auto fNPC = curMap->npcs.find({ci, cj});
					if (fPort != curMap->ports.end()) {
						// make sure ports and resource node aren't on the same block
						mvaddch(i, j, '^');
					}
					// draw resource node if it exists in this block
					else if (fResource != curMap->resources.end()) {
						// if the resource node has regrown
						if (fResource->second.regrowTime < frame) mvaddch(i, j, fResource->second.look);
					}
					// draw npc if it exists in this block
					else if (fNPC != curMap->npcs.end()) {
						mvaddch(i, j, '0');
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
		// drawing npc dialogue in talking mode
		if (isTalking) {
			mvaddstr(4, camWid + 10, curNPC->dialogues[curNPC->diaNum].words.c_str());
		}
		player.dispInventory(0, camWid + 20);
		// uploads drawing onto terminal
		refresh();
		int inp = getch();
		// currently talking to an NPC; all input is rerouted
		if (isTalking) {
			// try to advance dialogue
			if (inp == ' ') {
				// move onto next dialogue box. make sure it doesn't exceed maximum
				if (curNPC->diaNum < (int) curNPC->dialogues.size()) {
					// if this current dialogue box has a trigger lambda
					if (curNPC->dialogues[curNPC->diaNum].hasTrigger) {
						// calls lambda function to modify player
						// if lambda returns true (allows dialogue to advance)
						if (curNPC->dialogues[curNPC->diaNum].trigger(&player)) {
							curNPC->diaNum ++;
						}
					}
					// if there's not trigger condition to fullfill, advance to the next diaglogue
					else curNPC->diaNum ++;
				}
				// if reaches end of dialogue chain, end talking
				if (curNPC->diaNum == (int) curNPC->dialogues.size()) {
					curNPC->diaNum = 0;
					curNPC = nullptr;
					isTalking = false;
				}
			}
			// escape conversation
			else if (inp == '`') {
				// resetting npc and talking state, allowing player to move again
				curNPC = nullptr;
				isTalking = false;
			}
		}
		else {
			if (inp == 'w') player.move(-1, 0);
			else if (inp == 's') player.move(1, 0);
			else if (inp == 'a') player.move(0, -1);
			else if (inp == 'd') player.move(0, 1);
			// change player facing
			else if (inp == KEY_UP) player.faceI = -1, player.faceJ = 0;
			else if (inp == KEY_DOWN) player.faceI = 1, player.faceJ = 0;
			else if (inp == KEY_LEFT) player.faceI = 0, player.faceJ = -1;
			else if (inp == KEY_RIGHT) player.faceI = 0, player.faceJ = 1;
			// action depending on the player's facing. can be collecting resources or attacking
			else if (inp == ' ') player.act();
		}
	}
	endwin();
}

