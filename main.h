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

// forward declarations
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
