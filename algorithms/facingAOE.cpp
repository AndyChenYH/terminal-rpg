
#include <bits/stdc++.h>
using namespace std;

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
int main() {
	vector<vector<int>> a = {
		{1, 0, 0},
		{1, 1, 0},
		{1, 0, 0}
	};
	vector<vector<int>> right = a, up = rotateMatrix(a), left = rotateMatrix(up), down = rotateMatrix(left);
	vector<vector<int>> dr;
	int i = 5, j = 5;
	int faceI = -1, faceJ = 0;
	int relI, relJ;
	if (faceI == 0 && faceJ == 1) {
		relI = -1, relJ = 1;
		dr = right;
	}
	else if (faceI == -1 && faceJ == 0) {
		relI = -3, relJ = -1;
		dr = up;
	}
	else if (faceI == 0 && faceJ == -1) {
		relI = -1, relJ = -3;
		dr = left;
	}
	else if (faceI == 1 && faceJ == 0) {
		relI = 1, relJ = -1;
		dr = down;
	}
	else assert(false);
	relI += i, relJ += j;
	int N = a.size();
	vector<vector<int>> grid(10, vector<int>(10));
	for (int ii = 0; ii < N; ii ++) {
		for (int jj = 0; jj < N; jj ++) {
			printf(">>%d %d %d %d\n", relI + ii, relJ + jj, ii, jj);
			grid[relI + ii][relJ + jj] = dr[ii][jj];
		}
	}
	grid[i][j] = 2;
	for (vector<int> v : grid) {
		for (int d : v) printf("%d ", d);
		puts("");
	}
}
