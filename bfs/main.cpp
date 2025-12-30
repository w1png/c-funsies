#include <cstdlib>
#include <deque>
#include <map>
#include <unistd.h>
#include <vector>

using namespace std;

#define WALL_INT 1
#define EMPTY_INT 0
#define END_INT 3
#define CHECKED_INT 4

#define TOTAL_OBSTACLES 100

#define MATRIX_SIZE_Y 10
#define MATRIX_SIZE_X 50

class Maze {
public:
  vector<vector<int>> maze;
  Maze(pair<int, int> end = {-1, -1}) {
    this->maze =
        vector<vector<int>>(MATRIX_SIZE_Y, vector<int>(MATRIX_SIZE_X, 0));

    for (int i = 0; i < TOTAL_OBSTACLES; i++) {
      pair<int, int> curr = {rand() % MATRIX_SIZE_Y, rand() % MATRIX_SIZE_X};
      this->maze[curr.first][curr.second] = WALL_INT;
    }

    if (end.first == -1 && end.second == -1) {
      this->maze[rand() % MATRIX_SIZE_Y][rand() % MATRIX_SIZE_X] = END_INT;
    } else {
      this->maze[end.first][end.second] = END_INT;
    }
  }

#define WALL "█"
#define EMPTY "▒"
#define END "."
#define PLAYER "☺"
#define CHECKED "☑"

  void print(pair<int, int> playerPosition = {-1, -1}) {
    printf("\033[2J");
    for (int y = 0; y < MATRIX_SIZE_Y; y++) {
      for (int x = 0; x < MATRIX_SIZE_X; x++) {
        if (playerPosition.first == y && playerPosition.second == x) {
          printf("%s", PLAYER);
          continue;
        }

        if (this->maze[y][x] == WALL_INT) {
          printf("%s", WALL);
        } else if (this->maze[y][x] == END_INT) {
          printf("%s", END);
        } else if (this->maze[y][x] == CHECKED_INT) {
          printf("%s", CHECKED);
        } else {
          printf("%s", EMPTY);
        }
      }
      puts("");
    }

    usleep(5000);
  }
};

vector<pair<int, int>> solve(Maze &maze, pair<int, int> curr) {
  map<pair<int, int>, pair<int, int>> parents;
  parents[curr] = make_pair(-1, -1);
  deque<pair<int, int>> queue;
  maze.maze[curr.first][curr.second] = CHECKED_INT;
  queue.push_back(curr);

  while (!queue.empty()) {
    pair<int, int> curr = queue.front();
    maze.print(curr);
    if (maze.maze[curr.first][curr.second] == END_INT) {
      vector<pair<int, int>> path;
      pair<int, int> at = parents[curr];
      while (at != make_pair(-1, -1)) {
        path.push_back(at);
        at = parents[at];
      }
      reverse(path.begin(), path.end());

      return path;
    }

    pair<int, int> directions[4] = {
        {0, -1},
        {0, 1},
        {-1, 0},
        {1, 0},
    };

    for (auto &direction : directions) {
      pair<int, int> neighbour = {curr.first + direction.first,
                                  curr.second + direction.second};

      if (neighbour.first < 0 || neighbour.first > MATRIX_SIZE_Y - 1 ||
          neighbour.second < 0 || neighbour.second > MATRIX_SIZE_X - 1) {
        continue;
      }

      int cell = maze.maze[neighbour.first][neighbour.second];
      if (cell == WALL_INT || cell == CHECKED_INT) {
        continue;
      }

      if (cell != END_INT) {
        maze.maze[neighbour.first][neighbour.second] = CHECKED_INT;
      }
      queue.push_back(neighbour);
      parents[neighbour] = curr;
    }

    queue.pop_front();
  }

  fprintf(stderr, "No path found\n");
  exit(-1);
}

int main(void) {
  srand(time(NULL));

  Maze maze = Maze();

  auto path = solve(maze, {5, 5});

  Maze solvedMaze = Maze({path.back().first, path.back().second});
  for (auto &p : path) {
    if (p.first == path.back().first && p.second == path.back().second) {
      continue;
    }
    solvedMaze.maze[p.first][p.second] = CHECKED_INT;
  }
  solvedMaze.print();
  printf("End found at {%d,%d}", path.back().first, path.back().second);

  return 0;
}
