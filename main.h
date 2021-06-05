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
	// w is weapon, r is resource
	char type;
	// single character appearance
	char look;
	int damage;
	vector<vector<bool>> aoe;
	Item();
	Item(string name, char type, char look);
};


class Player {
	public:
	// coordinates and facing directional vectors
	int i, j, faceI, faceJ;
	int health;
	// item and amount
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

	Map();
	Map(string file);
	bool inBound(int i, int j);

};