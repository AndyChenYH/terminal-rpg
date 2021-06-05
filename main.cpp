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

// rotates a square matrix counter clockwise 90 degrees
vector<vector<int>> rotateMatrix(vector<vector<int>> mat) {
	int N = mat.size();
	for (int x = 0; x < N / 2; x++) {
		for (int y = x; y < N - x - 1; y++) {
			int temp = mat[x][y];
			mat[x][y] = mat[y][N - 1 - x];
			mat[y][N - 1 - x]
				= mat[N - 1 - x][N - 1 - y];
			mat[N - 1 - x][N - 1 - y]
				= mat[N - 1 - y][x];
			mat[N - 1 - y][x] = temp;
		}
	}
	return mat;
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
							coord1 = {stoi(img[i + 1]), stoi(img[i + 2])};
						}
						else if (img[i] == "<coord2>") {
							coord2 = {stoi(img[i + 1]), stoi(img[i + 2])};
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
							int x = stoi(img[i + 1]), y = stoi(img[i + 2]);
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
														// 1 is give to npc, 2 is receive from npc
														string item1 = img[i + 1], item2 = img[i + 3];
														// read in the give and receive amounts
														// use NONE to denote giving or receiving nothing
														int amount1 = stoi(img[i + 2]), amount2 = stoi(img[i + 4]);
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
							res.first.first = stoi(img[i + 1]);
							res.first.second = stoi(img[i + 2]);
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
/*
// basic map initialization
Map::Map(string description, int row, int col) : description(description), row(row), col(col) {
	data = vector<vector<Block>>(row, vector<Block>(col));
}
*/
// check if is in the bound of the map
bool Map::inBound(int i, int j) {
	return 0 <= i && i < row && 0 <= j && j < col;
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
// should be even numbers
const int camHei = 40, camWid = 40;

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
	if (!curMap->inBound(newI, newJ) || !curMap->data[newI][newJ].pass) return;
	i = newI, j = newJ;
	// seeing if block moved on is a portal
	auto fid = curMap->ports.find({i, j});
	if (fid != curMap->ports.end()) {
		// perform portal teleportation
		curMap = &maps.at(get<0>(fid->second));
		i = get<1>(fid->second);
		j = get<2>(fid->second);
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
	vector<vector<int>> right = inventory[hotBarNum].first.aoe, up = rotateMatrix(right), left = rotateMatrix(up), down = rotateMatrix(left);
	// width and height of N*N square matrix
	int N = right.size();
	// actual aoe for current facing
	vector<vector<int>> aoeDir;
	// relative postitioning of the top left corner of aoe matrix based on player facing
	int relI, relJ;
	// facing right
	if (faceI == 0 && faceJ == 1) {
		relI = -1, relJ = 1;
		aoeDir = right;
	}
	// facing up
	else if (faceI == -1 && faceJ == 0) {
		relI = -N, relJ = -1;
		aoeDir = up;
	}
	// facing left
	else if (faceI == 0 && faceJ == -1) {
		relI = -1, relJ = -N;
		aoeDir = left;
	}
	// facing down
	else if (faceI == 1 && faceJ == 0) {
		relI = 1, relJ = -1;
		aoeDir = down;
	}
	// shouldn't be any other facing direction
	else assert(false);


	if (inventory[hotBarNum].first.type == "tool") {
		Item &it = inventory[hotBarNum].first;
		// ii and jj and the locations in the aoeDir matrix
		for (int ii = 0; ii < N; ii ++) {
			for (int jj = 0; jj < N; jj ++) {
				// (i + relI + ii, j + relJ + jj) is the absolute coordinate of the matrix element on the world map
				int absI = i + relI + ii, absJ = j + relJ + jj;
				// if the location in the aoe matrix is 0, then it means that's an empty spot
				if (aoeDir[ii][jj] == 0) continue;
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
		}
	}
}
void Player::dispHotbar(int relI, int relJ) {
	// draw outline of hotbar
	drawImage(0, camWid + 20, hotbarBox);
	// draw items within outline
	for (int jj = 0; jj < min(4, int(inventory.size())); jj ++) {
		if (inventory[jj].first.name != "NONE") {
			mvaddstr(1 + relI, 1 + relJ + jj * 8, 
				(string(1, inventory[jj].first.look) + "   " + to_string(inventory[jj].second)).c_str()); 
		}
	}
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
			int N = img[i].size();
			it.aoe = vector<vector<int>>(N, vector<int>(N));
			for (int ii = 0; img[i] != "</aoe>"; ii ++, i ++) {
				for (int j = 0; j < N; j ++) {
					it.aoe[ii][j] = img[i][j] - '0';
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

	// loading preset items from txt file
	loadItems();
	loadMaps();
	// testing code that can be deleted later
	curMap = &maps.at("worldMap");
	player.addItem(items["basic_pickaxe"]);

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
						if (fResource->second.second < frame) mvaddch(i, j, fResource->second.first.look);
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
	}
	endwin();
}

