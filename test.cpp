#include <bits/stdc++.h>
#include <ncurses.h>
#include <unistd.h>
using namespace std;

int cc = 8;
int cp = 1;
void drawChar(int i, int j, char c, int r, int g, int b) {
	cc ++;
	init_color(cc, r, g, b);
	init_pair(cp, cc, COLOR_BLACK);
	cp ++;
	mvaddch(i, j, c);
}

int main() {

	initscr();
	cbreak();
	noecho();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	start_color();
	while (true) {
		usleep(20000);
		erase();
		mvaddstr(2, 3, "hello world");
		attroff(COLOR_PAIR(1));
		refresh();
	}
}
