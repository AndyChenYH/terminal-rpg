#include <bits/stdc++.h>
#include "main.h"
#include <ncurses.h>
#include <unistd.h>
using namespace std;

// output file for debugging
ofstream fout("out.txt");
// python2 style print function for debugging; outputs with fout
namespace __hidden__ { struct print { bool space; print() : space(false) {} ~print() { fout << endl; } template <typename T> print &operator , (const T &t) { if (space) fout << ' '; else space = true; fout << t; return *this; } }; }
#define print __hidden__::print(),
// trim white spaces
string trimWhite(const string& str, const string& whitespace = " \t") { const auto strBegin = str.find_first_not_of(whitespace); if (strBegin == string::npos) return ""; const auto strEnd = str.find_last_not_of(whitespace); const auto strRange = strEnd - strBegin + 1; return str.substr(strBegin, strRange); }
// split string by delimiter
vector<string> splitString(const string str, const string delim) { vector<string> tokens; size_t prev = 0, pos = 0; do { pos = str.find(delim, prev); if (pos == string::npos) pos = str.length(); string token = str.substr(prev, pos-prev); if (!token.empty()) tokens.push_back(token); prev = pos + delim.length(); } while (pos < str.length() && prev < str.length()); return tokens; }
// rotates a square matrix counter clockwise 90 degrees
vector<vector<int>> rotateMatrix(vector<vector<int>> mat) { int N = mat.size(); for (int x = 0; x < N / 2; x++) { for (int y = x; y < N - x - 1; y++) { int temp = mat[x][y]; mat[x][y] = mat[y][N - 1 - x]; mat[y][N - 1 - x] = mat[N - 1 - x][N - 1 - y]; mat[N - 1 - x][N - 1 - y] = mat[N - 1 - y][x]; mat[N - 1 - y][x] = temp; } } return mat; }

const vector<pair<int, int>> drs = {{-1, 0}, {0, -1}, {1, 0}, {0, 1}};

// loads image from txt file and returns as list of horizontal lines
vector<string> loadImage(string fil) {
	ifstream fin(fil);
	vector<string> res;
	string ln;
	while (getline(fin, ln)) {
		res.push_back(ln);
	}
	return res;
}
// similar to loadimage, except reads AML(andy's markup language) and strips white spaces
vector<string> loadAML(string fil) {
	ifstream fin(fil);
	vector<string> res;
	string ln;
	while (getline(fin, ln)) {
		ln = trimWhite(ln);
		if (ln != "") {
			res.push_back(ln);
		}
	}
	// check if the tags in the AML file matchs
	list<string> st;
	for (string s : res) {
		if (!(s[0] == '<' && s[int(s.size()) - 1] == '>')) continue;
		if (s[1] == '/') {
			string tmp(s.begin(), s.end());
			tmp.erase(tmp.begin() + 1);
			if (st.empty() || tmp != st.back()) {
				endwin();
				cout << "AML file mismatch: " << s << endl;
				exit(0);
			}
			st.pop_back();
		}
		else st.push_back(s);
	}
	if (!st.empty()) {
		endwin();
		cout << "AML file mismatch: " << st.back() << endl;
		exit(0);
	}
	return res;
}
// draws image onto screen with top left corner at (relI, relJ)
void drawImage(int relI, int relJ, vector<string> img) {
	for (int i = 0; i < (int) img.size(); i ++) {
		mvaddstr(relI + i, relJ, img[i].c_str());
	}
}
// load images before hand to save time
vector<string> hotbarBox = loadImage("assets/hotbarBox.txt");
vector<string> inventoryBox = loadImage("assets/inventoryBox.txt");

// should take more than a year to overflow integer size limit
int frame = 0;
// is player talking to npc?
bool isTalking = false;
// is player looking at inventory?
bool viewInventory = false;
// should be odd numbers, so player will be in middle
const int camHei = 21, camWid = 21;
// rectangular area in which things will be constantly refreshed/loaded
const int loadHei = 50, loadWid = 50;
// current color pair id; used in init_pair and attron(COLOR_PAIR()) to identify individual color pairs
// current color id: used in init_color to identify individual colors; numbers 0-7 are taken by original colors
int curColorPairId = 1, curColorId = 8;
// maps name of color to a color pair
map<string, int> colors = {
	{"default", -1},
	{"black", COLOR_BLACK},
	{"red", COLOR_RED},
	{"green", COLOR_GREEN},
	{"yellow", COLOR_YELLOW},
	{"blue", COLOR_BLUE},
	{"magenta", COLOR_MAGENTA},
	{"cyan", COLOR_CYAN},
	{"white", COLOR_WHITE},
};
map<string, int> colorPairs;
void makeColor(string name, int r, int g, int b) {
	assert(r < 1000 && g < 1000 && b < 1000 && curColorId < COLORS);
	init_color(curColorId, r, g, b);
	colors.insert({name, curColorId});
	curColorId ++;
}
void makeColorPair(string name, string foreground, string background) {
	init_pair(curColorPairId, colors[foreground], colors[background]);
	colorPairs.insert({name, curColorPairId});
	curColorPairId ++;
}

Block::Block() {
	pass = true;
	look = '.';
}
Block::Block(bool pass, char look) : pass(pass), look(look) { }

Item::Item() {}
// resource initialization
Item::Item(string name, string type, char look) : name(name), type(type), look(look) { }


// NONE item: represents empty space
// set to '~' for debugging purposes
const Item NONE_ITEM("NONE", "", '~');

// preset items mapped from item names
// allows reference to items just by name
map<string, Item> items;

// empty constructor 
Dialogue::Dialogue() {}
// by default, there's no trigger, since most dialogues are simply text/storytelling
Dialogue::Dialogue(string words) : words(words), hasTrigger(false) { }
// dialogue with trigger function (must return true to proceed and can modify player)
Dialogue::Dialogue(string words, function<bool(Player*)> trigger) 
	: words(words), hasTrigger(true), trigger(trigger) {}

NPC::NPC(string name, vector<Dialogue> dialogues) 
	: name(name), diaNum(0), dialogues(dialogues) { }

Enemy::Enemy(int health) : health(health) {}

// treemap to allow referencing maps by their names
map<string, Map> maps;

// initializing map first so portal can use it
Map::Map() {}
// read map from file
// html-style enclosure
// make sure there are no extra spaces
Map::Map(string file) {
	// loading file data as a bunch of lines (using loadAML function for convience)
	vector<string> img = loadAML(file);
	// current line in the file
	int i = 0;
	// img[i] is a line in the file
	for ( ; i < (int) img.size(); i ++) {
		// load basic look
		// also initializes size of map (row and column)
		if (img[i] == "<basicLook>") {
			i ++;
			// initialize row and column using basic look's rectangular dimension
			row = 0, col = img[i].size();
			for ( ; img[i] != "</basicLook>"; i ++) {
				// add new row to map data
				data.push_back(vector<Block>(col));
				for (int j = 0; j < col; j ++) {
					data[row][j].look = img[i][j];
				}
				row ++;
			}
		}
		// read in list of portals
		else if (img[i] == "<portals>") {
			for ( ; img[i] != "</portals>"; i ++) {
				// read in singular portal
				if (img[i] == "<portal>") {
					pair<int, int> coord1, coord2;
					string mapTo;
					for ( ; img[i] != "</portal>"; i ++) {
						if (img[i] == "<coord1>") {
							vector<string> sp = splitString(img[i + 1], " ");
							coord1 = {stoi(sp[0]), stoi(sp[1])};
						}
						else if (img[i] == "<coord2>") {
							vector<string> split = splitString(img[i + 1], " ");
							coord2 = {stoi(split[0]), stoi(split[1])};
						}
						else if (img[i] == "<mapTo>") {
							mapTo = img[i + 1];
						}
					}
					ports.insert({coord1, {mapTo, coord2.first, coord2.second}});
				}
			}
		}
		// read in list npcs
		else if (img[i] == "<npcs>") {
			// coordinate of npc on map, used as key in treemap
			pair<int, int> coord;
			for ( ; img[i] != "</npcs>"; i ++) {
				// read in single npc
				if (img[i] == "<npc>") {
					// npc object for assembling and adding to list of npcs
					NPC npc;
					// the dialog number always starts at 0
					npc.diaNum = 0;
					for ( ; img[i] != "</npc>"; i ++) {
						// read in npc coordinates x, y in two lines
						if (img[i] == "<coord>") {
							// change the integers into string and put them in coordinate pair
							vector<string> sp = splitString(img[i + 1], " ");
							int x = stoi(sp[0]), y = stoi(sp[1]);
							coord = {x, y};
						}
						else if (img[i] == "<name>") {
							npc.name = img[i + 1];
						}
						// read in list of dialogues
						else if (img[i] == "<dialogues>") {
							for ( ; img[i] != "</dialogues>"; i ++) {
								// starting tag of single dialogue
								if (img[i] == "<dialogue>") {
									// dialogue object to eventually assemble and add to list of dialogues
									Dialogue dialogue;
									// unless reads in trigger function later, false by default
									dialogue.hasTrigger = false;
									// loop until the end of current singular dialogue
									for ( ; img[i] != "</dialogue>"; i ++) {
										if (img[i] == "<words>") {
											dialogue.words = img[i + 1];
										}
										else if (img[i] == "<trigger>") {
											// if sees trigger tag, mean that the dialogue has trigger function
											dialogue.hasTrigger = true;
											// type of trigger: choose from pre-set
											string type = "";
											for ( ; img[i] != "</trigger>"; i ++) {
												if (img[i] == "<type>") {
													type = img[i + 1];
												}
												else if (img[i] == "<data>") {
													// a trade between player and NPC
													if (type == "trade") {
														// item to give and amount
														vector<string> sp1 = splitString(img[i + 1], " ");
														vector<string> sp2 = splitString(img[i + 2], " ");
														// 1 is give to npc, 2 is receive from npc
														string item1 = sp1[0], item2 = sp2[0];
														// read in the give and receive amounts
														// use NONE to denote giving or receiving nothing
														int amount1 = stoi(sp1[1]), amount2 = stoi(sp2[1]);
														// have to use capture-by-value because the variables would go outside of scope
														dialogue.trigger = [=] (Player *pl) -> bool {
															// takeItem returns true if item taken successfully
															bool res = item1 == "NONE" ? true : pl->takeItem(items[item1], amount1);
															// if successfully took item, player can now receive reward
															if (res && item2 != "NONE") pl->addItem(items[item2], amount2);
															// the returned value is used to determine whether dialogue can advance
															return res;
														};

													}
												}
											}
										}
									}
									// add assembled dialogue to the list
									npc.dialogues.push_back(dialogue);
								}
							}
						}
					}
					// insert assebled npc into the treemap of npcs within the map
					npcs.insert({coord, npc});
				}
			}
		}
		// read in passability
		else if (img[i] == "<passability>") {
			i ++;
			// simple nested for loop to read in passibility
			// i is index within img, ii is index within data
			for (int ii = 0; ii < row; ii ++, i ++) {
				for (int j = 0; j < col; j ++) {
					data[ii][j].pass = img[i][j] == '0' ? true : false;
				}
			}
		}
		else if (img[i] == "<resources>") {
			i ++;
			// looping through all resources
			for ( ; img[i] != "</resources>"; i ++) {
				// single resource instance
				if (img[i] == "<resource>") {
					// read the single object and add it to map's resources
					i ++;
					// set up the <coord, <item, regrowTime>> to be modified & returned
					pair<pair<int, int>, pair<Item, int>> res;
					// set the default regrowtime to 0
					res.second.second = 0;
					// read until end of resource tag
					for ( ; img[i] != "</resource>"; i ++) {
						// read in coordinates on two lines
						if (img[i] == "<coord>") {
							vector<string> sp = splitString(img[i + 1], " ");
							res.first.first = stoi(sp[0]);
							res.first.second = stoi(sp[1]);
						}
						// read in item name
						// which can be referenced with the "items" treemap to get an Item object
						else if (img[i] == "<item>") {
							i ++;
							res.second.first = items[img[i]];
						}
					}
					// put the resource node with all the assembled information into the list
					resources.insert(res);
				}
			}
		}
	}
	
}
// check if is in the bound of the map
bool Map::inBound(int i, int j) {
	return 0 <= i && i < row && 0 <= j && j < col;
}
// sI and sJ are player coordinates, in absolute map position
// top, left, hei, and wid specify the rectangular field in which bfs is done (eg: current camera view)
void Map::enemyPathfind(int sI, int sJ, int top, int left, int hei, int wid) {
	assert(0 <= top && 0 <= left && top + hei <= row && left + wid <= col);
	// coordinates of visited and parent are relative to the rectangular field
	vector<vector<bool>> vis(hei, vector<bool>(wid, false));
	vector<vector<pair<int, int>>> parent(hei, vector<pair<int, int>>(wid));
	for (int i = 0; i < hei; i ++) {
		for (int j = 0; j < wid; j ++) parent[i][j] = {i, j};
	}
	// coordinates in queue are relative to the field
	list<pair<int, int>> q;
	// add the initial starting position into the queue
	q.push_back({sI - top, sJ - left});
	while (!q.empty()) {
		int i = q.front().first, j = q.front().second;
		q.pop_front();
		for (pair<int, int> d : drs) {
			int ni = i + d.first, nj = j + d.second;
			if (0 <= ni && ni < hei && 0 <= nj && nj < wid && data[top + ni][left + nj].pass) {
				if (!vis[ni][nj]) {
					q.push_back({ni, nj});
					vis[ni][nj] = true;
					parent[ni][nj] = {i, j};
				}
			}
		}
	}

	map<pair<int, int>, Enemy> newPos;
	for (pair<pair<int, int>, Enemy> pp : enemies) {
		// absolute position
		pair<int, int> absPos = pp.first;
		// relative position
		pair<int, int> relPos = {absPos.first - top, absPos.second - left};
		bool enemyIn = 0 <= relPos.first && relPos.first < hei && 0 <= relPos.second && relPos.second < wid;
		if (!enemyIn) {
			newPos.insert(pp);
			continue;
		}

		// relative parent
		pair<int, int> relPar = parent[relPos.first][relPos.second];
		// absolute parent
		pair<int, int> absPar = {relPar.first + top, relPar.second + left};

		if (newPos.find(absPar) != newPos.end() || enemies.find(absPar) != enemies.end()
				|| absPar == make_pair(sI, sJ)) newPos.insert(pp);
		else newPos.insert({absPar, pp.second});
	}
	enemies = newPos;
}

void loadMaps() {
	maps.insert({"worldMap", Map("assets/worldMap.txt")});
	maps.insert({"inn", Map("assets/inn.txt")});
	maps.insert({"church", Map("assets/church.txt")});
}
// current map
Map *curMap;
// current NPC the player is talking to
NPC *curNPC;

Player::Player(int i, int j) : i(i), j(j), health(10), hotBarNum(0) {
	faceI = 0, faceJ = 1;
	inventory = vector<pair<Item, int>>(20, {NONE_ITEM, 0});
}
// called every frame to check quest completion status & give reward
void Player::checkQuests() {
	for (int ii = 0; ii < (int) quests.size(); ii ++) {
		// quest returns bool based on completed or not
		bool res = quests[ii](this);
		// if completed, erase quest from list 
		if (res) {
			quests.erase(quests.begin() + ii);
			ii --;
		}
	}
}
// directional vectors
void Player::move(int di, int dj) {
	int newI = i + di, newJ = j + dj;
	// if moving out of bounds or into an unpassable block, don't move
	// don't move onto an enemy
	if (!curMap->inBound(newI, newJ) || !curMap->data[newI][newJ].pass
			|| curMap->enemies.find({newI, newJ}) != curMap->enemies.end()) return;
	i = newI, j = newJ;
	// seeing if block moved on is a portal
	auto fPort = curMap->ports.find({i, j});
	if (fPort != curMap->ports.end()) {
		// perform portal teleportation
		curMap = &maps.at(get<0>(fPort->second));
		i = get<1>(fPort->second);
		j = get<2>(fPort->second);
	}

}
void Player::addItem(Item newIt, int num = 1) {
	// does the player's inventory already have at least one of the resource?
	// if so, just increment the amount
	for (pair<Item, int> &pp : inventory) {
		if (pp.first.name == newIt.name) {
			pp.second += num;
			return;
		}
	}
	// if the item isn't already in the inventory, replace an empty spot with it
	for (pair<Item, int> &pp : inventory) {
		if (pp.first.name == "NONE") {
			pp = {newIt, num};
			return;
		}
	}
	// item couldn't be added; probably because inventory is full
	assert(false);
}
// if item exists in inventory, take it out and return true
// otherwise return false
bool Player::takeItem(Item takeIt, int num = 1) {
	for (int ii = 0; ii < (int) inventory.size(); ii ++) {
		// if has item and has enough amount
		if (takeIt.name == inventory[ii].first.name && inventory[ii].second >= num) {
			inventory[ii].second -= num;
			// if item amount is zero, remove it from inventory
			if (inventory[ii].second == 0) inventory[ii].first = NONE_ITEM;
			return true;
		}
	}
	return false;
}
void Player::act() {
	// coordinates of block the player is targeting
	// this does not account for aoe; it's only one block away from player
	int ci = i + faceI, cj = j + faceJ;

	// see if player has chosen to interact with an npc
	auto fNPC = curMap->npcs.find({ci, cj});
	if (fNPC != curMap->npcs.end()) {
		// set talking mode
		isTalking = true;
		// set current NPC that the player is talking to
		curNPC = &(fNPC->second);
	}

	// *** below are all actions involving weapons or tools ***
	if (inventory[hotBarNum].first.type == "NONE") return;

	// rotated aoe for each facing
	vector<vector<int>> right = inventory[hotBarNum].first.aoe, up = rotateMatrix(right),
	  	left = rotateMatrix(up), down = rotateMatrix(left);
	// width and height of N*N square matrix
	int N = right.size();
	// actual aoe for current facing
	vector<vector<int>> aoeDir;
	// relative postitioning of the top left corner of aoe matrix based on player facing
	// TODO: shouldn't be 1 or -1, should be half of aoe size
	int relI, relJ;
	// facing right
	if (faceI == 0 && faceJ == 1) {
		relI = -N / 2, relJ = N / 2;
		aoeDir = right;
	}
	// facing up
	else if (faceI == -1 && faceJ == 0) {
		relI = -N, relJ = -N / 2;
		aoeDir = up;
	}
	// facing left
	else if (faceI == 0 && faceJ == -1) {
		relI = -N / 2, relJ = -N;
		aoeDir = left;
	}
	// facing down
	else if (faceI == 1 && faceJ == 0) {
		relI = N / 2, relJ = -N / 2;
		aoeDir = down;
	}
	// shouldn't be any other facing direction
	else assert(false);

	// ii and jj and the locations in the aoeDir matrix
	for (int ii = 0; ii < N; ii ++) {
		for (int jj = 0; jj < N; jj ++) {
			// (i + relI + ii, j + relJ + jj) is the absolute coordinate of the matrix element on the world map
			int absI = i + relI + ii, absJ = j + relJ + jj;
			// if the location in the aoe matrix is 0, then it means that's an empty spot, and doesn't do any damage there
			if (aoeDir[ii][jj] == 0) continue;
			// if player is currently holding a tool type item in their selected hotbar slot
			if (inventory[hotBarNum].first.type == "tool") {
				// search the map coordinates for any resources
				auto fResource = curMap->resources.find({absI, absJ});
				// see whether the target block has a resource and current time is past its regrow time
				if (fResource != curMap->resources.end() && fResource->second.second < frame) {
					// aoeDir[ii][[jj] is the amount the tool can harvest in the given spot in the aoe matrix
					// adding the resource and amount to the player's inventory
					addItem(fResource->second.first, aoeDir[ii][jj]);
					// set resource's regrow time to 200 frames in the future
					fResource->second.second = frame + 200;
				}
			}
			else if (inventory[hotBarNum].first.type == "weapon") {
				// search the map for enemy at that locatioin
				auto fEnemy = curMap->enemies.find({absI, absJ});
				// if enemy at that location exists in the list of enemies
				if (fEnemy != curMap->enemies.end()) {
					// health subtract the damage in the spot the aoe hit
					fEnemy->second.health -= aoeDir[ii][jj];
					// if health falls below zero, erase enemy from list of enemies
					if (fEnemy->second.health <= 0) curMap->enemies.erase(fEnemy);
				}
			}
		}
	}
}
void Player::dispHotbar(int relI, int relJ) {
	// draw outline of hotbar
	drawImage(0, camWid + 20, hotbarBox);
	// draw items within outline
	for (int jj = 0; jj < min(4, int(inventory.size())); jj ++) {
		// if item is NONE (meaning inventory is empty at that spot), don't draw anything
		if (inventory[jj].first.name != "NONE") {
			mvaddstr(1 + relI, 1 + relJ + jj * 8, 
				(string(1, inventory[jj].first.look) + "   " + to_string(inventory[jj].second)).c_str()); 
		}
	}
	// draw current selected slot pointer
	mvaddch(relI + 3, relJ + 3 + hotBarNum * 8, '^');
}
// relative coordinates to allow easy translation of object
void Player::dispInventory(int relI, int relJ) {
	// draw outline of inventory
	drawImage(relI, relJ, inventoryBox);
	// current index inside player's inventory
	int ci = 0;
	// ii and jj are the location in the inventory matrix
	// could tecnically extend forever
	for (int ii = 0; ci < (int) inventory.size(); ii ++) {
		for (int jj = 0; ci < (int) inventory.size() && jj < 4; jj ++) {
			// make sure it's not a NONE type item (denoting empty space)
			if (inventory[ci].first.name != "NONE") {
				// draw the character look of the item and amount
				mvaddstr(1 + relI + ii * 2, 1 + relJ + jj * 8, 
						(string(1, inventory[ci].first.look) + "   " + to_string(inventory[ci].second)).c_str()); 
			}
			ci ++;
		}
	}
}
// load a bunch of preset items from assets/items.txt
void loadItems() {
	// also load in the special none item type
	items.insert({"NONE", NONE_ITEM});
	vector<string> img = loadAML("assets/items.txt");
	// which line in the file
	int i = 0;
	Item it;
	for ( ; i < (int) img.size(); i ++) {
		// start of items object tag
		if (img[i] == "<item>") { it = Item(); }
		// if end of item tag, put the assembled item into the "items" treemap
		else if (img[i] == "</item>") { items.insert({it.name, it}); }
		// reading in other item attributes
		else if (img[i] == "<name>") { it.name = img[i + 1]; }
		else if (img[i] == "<type>") { it.type = img[i + 1]; }
		else if (img[i] == "<look>") { it.look = img[i + 1][0]; }
		// read in aoe pattern
		else if (img[i] == "<aoe>") {
			i ++;
			int N = splitString(img[i], " ").size();
			it.aoe = vector<vector<int>>(N, vector<int>(N));
			// i is absolute line position in the data file, ii is the relative row position in the aoe matrix
			for (int ii = 0; img[i] != "</aoe>"; ii ++, i ++) {
				vector<string> sp = splitString(img[i], " ");
				for (int j = 0; j < N; j ++) {
					it.aoe[ii][j] = stoi(sp[j]);
				}
			}
		}
	}
}
Player player(5, 5);

int main() {
	initscr();
	cbreak();
	// allows getch() to get input at any time, without waiting for input
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	noecho();
	// allows using -1 as natural terminal background color
	use_default_colors();
	start_color();

	// loading preset items from txt file
	loadItems();
	loadMaps();
	// testing code that can be deleted later
	curMap = &maps.at("worldMap");
	player.addItem(items["basic_pickaxe"]);
	player.addItem(items["basic_knife"]);
	curMap->enemies.insert({{7, 2}, Enemy(10)});
	curMap->enemies.insert({{20, 20}, Enemy(10)});
	curMap->enemies.insert({{20, 21}, Enemy(10)});
	makeColorPair("blue default", "blue", "default");
	makeColorPair("green default", "green", "default");

	while (true) {
		// 50 refreshes a second
		usleep(20000);
		frame ++;
		player.checkQuests();
		// clears screen of any output before next cycle; ncurses optimizes which characters to change
		erase();
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
				}
				// draw the sparse things stored as treemaps
				else if (curMap->inBound(ci, cj)) {
					// see if a port or resource node or npc has to be drawn
					auto fPort = curMap->ports.find({ci, cj});
					auto fResource = curMap->resources.find({ci, cj});
					auto fNPC = curMap->npcs.find({ci, cj});
					auto fEnemy = curMap->enemies.find({ci, cj});
					if (fPort != curMap->ports.end()) {
						// make sure ports and resource node aren't on the same block
						mvaddch(i, j, '^');
					}
					// draw resource node if it exists in this block
					else if (fResource != curMap->resources.end()) {
						// if the resource node has regrown
						if (fResource->second.second < frame) {
							attron(COLOR_PAIR(colorPairs["green default"]));
							mvaddch(i, j, fResource->second.first.look);
							attroff(COLOR_PAIR(colorPairs["green default"]));
						}
					}
					// draw npc if it exists in this block
					else if (fNPC != curMap->npcs.end()) {
						mvaddch(i, j, '0');
					}
					// draw enemy if it exists within this block
					else if (fEnemy != curMap->enemies.end()) {
						attron(COLOR_PAIR(colorPairs["blue default"]));
						mvaddch(i, j, 'E');
						attroff(COLOR_PAIR(colorPairs["blue default"]));
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
		// drawing player inventory
		if (viewInventory) player.dispInventory(0, camWid + 20);
		else {
			player.dispHotbar(0, camWid + 20);
		}

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
		else if (viewInventory) {
			// 'e' toggles inventory viewing, so pressing it again turns it off
			if (inp == 'e') {
				viewInventory = false;
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
			// change currrent hotbar slot
			else if ('1' <= inp && inp <= '4') {
				// converted it to zero-indexed
				player.hotBarNum = inp - 1 - '0';
			}
			// action depending on the player's facing. can be collecting resources or attacking
			else if (inp == ' ') player.act();
			// turn viewing inventory on and off
			else if (inp == 'e') viewInventory = !viewInventory;
		}
		// enemy pathfinding
		if (frame % 20 == 0) {
			// calculate rectangular space that bfs has to search through based on loadHei and loadWid
			// only search through this area speeds up computation
			int top = max(0, player.i - loadHei / 2), left = max(0, player.j - loadWid / 2);
			int bottom = min(curMap->row, top + loadHei + 1), right = min(curMap->col, left + loadWid + 1);
			curMap->enemyPathfind(player.i, player.j, top, left, bottom - top, right - left);
		}

	}
	endwin();
}

