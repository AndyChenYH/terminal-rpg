#include <bits/stdc++.h>
using namespace std;

class Block {
	public:
	bool pass;
	char look;
	Block();
	Block(bool pass, char look);
};

class Item {
	public:
	string name;
	// eg: weapon, resource
	string type;
	// single character appearance
	char look;
	// area of effect & damage at each location
	// must be a N*N matrix, where N is odd, so that it can be properly rotated and player can stand in middle
	// for weapons, the aoe numbers represent damage bonus
	// for resources, the aoe numbers represents the harvested amount
	// 0 means the spot is not affected by the aoe, any other positive number means affected
	vector<vector<int>> aoe;
	Item();
	Item(string name, string type, char look);
};


class Player {
	public:
	// coordinates and facing directional vectors
	int i, j, faceI, faceJ;
	int health;
	// current hotbar item
	int hotBarNum;
	// item and amount
	// set size of 20
	vector<pair<Item, int>> inventory;
	// quest lambda functions
	vector<function<bool(Player*)>> quests;
	Player(int i, int j);
	void checkQuests();
	void move(int di, int dj);
	void addItem(Item newIt, int num);
	bool takeItem(Item takeIt, int num);
	void act();
	void dispHotbar(int relI, int relJ);
	void dispInventory(int relI, int relJ);
};

// single dialogue/text box popup
class Dialogue {
	public:
	string words;
	bool hasTrigger;
	// when this dialogue is passed, modifies the player such as giving them a quest or items
	// return true if dialogue is allowed to advance (eg: player has met a certain condition)
	function<bool(Player*)> trigger;
	Dialogue();
	Dialogue(string words);
	Dialogue(string words, function<bool(Player*)> trigger);
};
class NPC {
	public:
	string name;
	int diaNum;
	// linear dialogue consisting of words, and a function that runs when player presses enter
	// can do things such as give player items or quests
	vector<Dialogue> dialogues;
	NPC() {}
	NPC(string name, vector<Dialogue> dialogues);
};
class Enemy {
	public:
	Enemy();
};
class Map {
	public:
	string description;
	int row, col;
	// map data
	vector<vector<Block>> data;
	// portals translates current map's coordinates to another map's coordinates
	// another map's name is represent by string; can be reference with "maps" treemap
	map<pair<int, int>, tuple<string, int, int>> ports;
	// coordinates of resource nodes, mapped to Item and their regrow times
	map<pair<int, int>, pair<Item, int>> resources;
	// coordinates of NPCs
	map<pair<int, int>, NPC> npcs;
	// coordinates of enemies
	map<pair<int, int>, Enemy> enemies;

	Map();
	Map(string file);
	bool inBound(int i, int j);
	// starting i, starting j, rectangular field in which to operate the path finding algorithm
	void enemyPathfind(int sI, int sJ, int top, int left, int hei, int wid);
};
