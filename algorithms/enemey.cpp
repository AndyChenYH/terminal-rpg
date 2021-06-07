#include <bits/stdc++.h>
#include <ncurses.h>
#include <unistd.h>
using namespace std;

int main() {

	initscr();
	cbreak();
	noecho();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	while (true) {
		usleep(20000);
		erase();
		mvaddch(2, 3, 'h');
		refresh();
	}
}
