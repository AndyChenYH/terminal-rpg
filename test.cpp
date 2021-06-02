#include <bits/stdc++.h>
using namespace std;

string trim(const string& str,
                 const string& whitespace = " \t")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

int main() {
	string s = " 				hello world				 ";
	cout << trim(s) << endl;
}
