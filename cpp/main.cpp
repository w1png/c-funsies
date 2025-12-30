#include <cstdlib>
#include <deque>
#include <unistd.h>
#include <vector>

using namespace std;

#define WALL_INT 1
#define EMPTY_INT 0
#define PLAYER_INT 2
#define END_INT 3
#define CHECKED_INT 4

#define TOTAL_OBSTACLES 30

class Maze {
public:
  vector<vector<int>> maze;
  Maze() {
    this->maze = vector<vector<int>>(10, vector<int>(10, 0));

    for (int i = 0; i < TOTAL_OBSTACLES; i++) {
      pair<int, int> curr = {rand() % 10, rand() % 10};
      this->maze[curr.first][curr.second] = WALL_INT;
    }

    this->maze[rand() % 9][rand() % 9] = END_INT;
  }

#define WALL "█"
#define EMPTY "▒"
#define END "."
#define PLAYER "☺"
#define CHECKED "☑"

  void print() {
    printf("\033[2J");
    for (int y = 0; y < 10; y++) {
      for (int x = 0; x < 10; x++) {
        if (this->maze[y][x] == WALL_INT) {
          printf("%s", WALL);
        } else if (this->maze[y][x] == PLAYER_INT) {
          printf("%s", PLAYER);
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

    usleep(10000);
  }
};

pair<int, int> solve(Maze &maze, pair<int, int> curr) {
  if (maze.maze[curr.first][curr.second] == END_INT) {
    return curr;
  }

  deque<pair<int, int>> queue;
  queue.push_back(curr);

  while (!queue.empty()) {
    pair<int, int> curr = queue.front();
    maze.maze[curr.first][curr.second] = CHECKED_INT;
    maze.print();

    pair<int, int> neighbours[8] = {
        // {curr.first - 1, curr.second - 1},
        {curr.first - 1, curr.second},
        // {curr.first - 1, curr.second + 1},
        {curr.first, curr.second - 1},
        {curr.first, curr.second + 1},
        // {curr.first + 1, curr.second - 1},
        {curr.first + 1, curr.second},
        // {curr.first + 1, curr.second + 1},
    };

    for (int i = 0; i < 8; i++) {
      pair<int, int> neighbour = neighbours[i];
      if (neighbour.first < 0 || neighbour.first > 9 || neighbour.second < 0 ||
          neighbour.second > 9) {
        continue;
      }

      if (maze.maze[neighbour.first][neighbour.second] == WALL_INT ||
          maze.maze[neighbour.first][neighbour.second] == CHECKED_INT) {
        continue;
      }

      if (maze.maze[neighbour.first][neighbour.second] == END_INT) {
        return neighbour;
      }

      queue.push_back(neighbour);
    }

    queue.pop_front();
  }

  fprintf(stderr, "No path found\n");
  exit(-1);
}

int main(void) {
  srand(time(NULL));

  Maze maze = Maze();

  pair<int, int> end = solve(maze, {1, 1});
  printf("End found at {%d,%d}", end.first, end.second);

  return 0;
}
