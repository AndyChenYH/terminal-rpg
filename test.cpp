#include <bits/stdc++.h>
using namespace std;

// rotates a square matrix counter clockwise 90 degrees
template <typename T> vector<vector<T>> rotateMatrix(vector<vector<T>> mat) { int N = mat.size(); for (int x = 0; x < N / 2; x++) { for (int y = x; y < N - x - 1; y++) { T temp = mat[x][y]; mat[x][y] = mat[y][N - 1 - x]; mat[y][N - 1 - x] = mat[N - 1 - x][N - 1 - y]; mat[N - 1 - x][N - 1 - y] = mat[N - 1 - y][x]; mat[N - 1 - y][x] = temp; } } return mat; }

int main() {
	vector<vector<string>> mat = {
		{"a", "b", "c"},
		{"d", "e", "f"},
		{"g", "h", "i"},
	};
	vector<vector<string>> rot = rotateMatrix(mat);
	for (vector<string> v : rot) {
		for (string s : v) cout << s << " ";
		puts("");
	}
}
