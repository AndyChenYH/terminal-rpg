#include <bits/stdc++.h>
#include <ncurses.h>
#include <unistd.h>
using namespace std;

int cc = 1;
short n = 8;
map<string, int> colors = {
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
	assert(r < 1000 && g < 1000 && b < 1000 && n < COLORS);
	init_color(n, r, g, b);
	colors.insert({name, n});
	n ++;
}
void makeColorPair(string name, string foreground, string background) {
	init_pair(cc, colors[foreground], colors[background]);
	colorPairs.insert({name, cc});
	cc ++;
}
int main() {
	initscr();
	cbreak();
	noecho();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	use_default_colors();
	start_color();
	makeColor("mured", 900, 200, 700);
	makeColor("agreen", 200, 900, 300);
	makeColorPair("blueblack", "blue", "black");
	makeColorPair("muredblack", "mured", "agreen");
	while (true) {
		usleep(20000);
		erase();
		attron(COLOR_PAIR(colorPairs["muredblack"]));
		mvaddstr(10, 10, "hello");
		attroff(COLOR_PAIR(colorPairs["muredblack"]));
		refresh();
	}
}
