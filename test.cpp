#include <bits/stdc++.h>
using namespace std;

vector<string> splitString(const string str, const string delim) { vector<string> tokens; size_t prev = 0, pos = 0; do { pos = str.find(delim, prev); if (pos == string::npos) pos = str.length(); string token = str.substr(prev, pos-prev); if (!token.empty()) tokens.push_back(token); prev = pos + delim.length(); } while (pos < str.length() && prev < str.length()); return tokens; }

int main() {
	vector<string> chat = {"hello world my friends joe moma is not very cool", "your mom is extremely gay and I and not gay", "so true more true things have never been spoken before lmao", "my dad gay", "i like playing the guitar because it makes soothing country music and is very relaxing to listen to", "This is a string of special characters that translate to clear the screen command.", "If you want a solution that will work on Windows, Mac & Linux/UNIX, you will need to come up with your own implementation. I do not believe that there is a single way to do it that works on all platforms."};
	const int chatHei = 10, chatWid = 50;
	vector<string> lines;
	lines.push_back("");
	for (string s : chat) {
		vector<string> ss = splitString(s, " ");
		for (int i = 0; i < (int) ss.size(); i ++) {
			string wd = ss[i];
			if (!(lines.back().size() + wd.size() < chatWid)) {
				lines.push_back("");
			}
			lines.back() += wd + " ";
		}
		lines.push_back("");
	}
	for (int i = max((int) lines.size() - chatHei, 0); i < (int) lines.size(); i ++) {
		cout << lines[i] << endl;
	}
}
