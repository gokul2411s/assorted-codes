#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <math.h>
#include <queue>
#include <set>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <vector>

using namespace std;

#define _mp make_pair

#define PI pair<int, int>
#define PPI pair< int, PI >

#define SPI set< PI >

#define VPI vector< PI >
#define VVPI vector< VPI >
#define VPPI vector< PPI >
#define PQPPI priority_queue< PPI, VPPI, greater< PPI > >

#define VI vector<int>
#define VVI vector< VI >
#define VC vector<char>
#define VVC vector< VC >
#define VB vector<bool>
#define VVB vector< VB >

int euclidean(int x1, int y1, int x2, int y2) {
  return (int)(sqrt((y2-y1)*(y2-y1) + (x2-x1)*(x2-x1)));
}

int naive(int x1, int y1, int x2, int y2) { return 0; }

class Board {
 public:
  Board(int N, int M) : N_(N), M_(M), board_(N) {
    for (int i = 0; i < N; i++) {
      board_[i].resize(M);
      for (int j = 0; j < M; j++) {
        board_[i][j] = rand() % 5 ? '_' : 'x';
      }
    }
  }

  int num_rows() const { return N_; }
  int num_cols() const { return M_; }
  bool obstacle(PI const & p) const { return board_[p.first][p.second] == 'x'; }
  bool contains(PI const & p) const { return p.first >= 0 && p.first < N_ && p.second >= 0 && p.second < M_; }

 private:
  int N_;
  int M_;
  VVC board_;
};

class Player {
 public:
  Player(Board const & board, int (*heuristic_fn)(int, int, int, int)) :
    board_(board),
    heuristic_fn_(heuristic_fn),
    visited_(board_.num_rows()),
    parent_(board_.num_rows()),
    distance_(board_.num_rows()) {
    for (int i = 0; i < board_.num_rows(); i++) {
      visited_[i].resize(board_.num_cols());
      parent_[i].resize(board_.num_cols());
      distance_[i].resize(board_.num_cols());
    }
  }

  VPI plan(PI tom, PI jerry) {
    reset();
    PQPPI work_queue; work_queue.push(_mp(heuristic_fn_(tom.second, tom.first, jerry.second, jerry.first), _mp(tom.first, tom.second)));
    distance_[tom.first][tom.second] = 0;
    while (!work_queue.empty()) {
      PPI const & work = work_queue.top();
      int dist = work.first;
      int i = work.second.first, j = work.second.second;
      work_queue.pop();

      if (visited_[i][j]) { continue; }
      visited_[i][j] = true;

      if (i == jerry.first && j == jerry.second) { break; }

      for (int k = -1; k <= 1; k++) {
        for (int l = -1; l <= 1; l++) {
          int row = i + k, col = j + l;
          if (!board_.contains(_mp(row, col))) { continue; }
          if (board_.obstacle(_mp(row, col))) { continue; }
          if (row == i && col == j) { continue; }
          if (!visited_[row][col]) {
            if (distance_[row][col] > distance_[i][j] + 1) {
              distance_[row][col] = distance_[i][j] + 1;
              work_queue.push(_mp(distance_[row][col] + heuristic_fn_(col, row, jerry.second, jerry.first), _mp(row, col)));
              parent_[row][col].first = i;
              parent_[row][col].second = j;
            }
          }
        }
      }
    }

    if (visited_[jerry.first][jerry.second]) {
      path_.push_back(jerry);
      while (true) {
        PI & p = parent_[jerry.first][jerry.second];
        path_.push_back(p);
        if (p.first == tom.first && p.second == tom.second) { break; }
        jerry.first = p.first; jerry.second = p.second;
      }
    }
    reverse(path_.begin(), path_.end());
    return path_;
  }

  bool explored(PI const & p) const { return visited_[p.first][p.second]; }

 private:
  Board board_;
  int (*heuristic_fn_) (int, int, int, int);
  VVB visited_;
  VVPI parent_;
  VVI distance_;
  VPI path_;

  void reset() {
    for (int i = 0; i < board_.num_rows(); i++) {
      fill(visited_[i].begin(), visited_[i].end(), false);
      fill(distance_[i].begin(), distance_[i].end(), numeric_limits<int>::max());
    }
    path_.clear();
  }
};

class Game {
 public:
  Game(Board const & board, Player const & player) : board_(board), player_(player) {
    int sx, sy;
    do {
      sx = rand() % board_.num_cols();
      sy = rand() % board_.num_rows();
    } while (!board_.obstacle(_mp(sy, sx)));
    tom_.first = sy;
    tom_.second = sx;

    int max_euclidean = euclidean(0, 0, board_.num_cols() - 1, board_.num_rows() - 1);
    int tx, ty;
    do {
      tx = rand() % board_.num_cols();
      ty = rand() % board_.num_rows();
    } while (!board_.obstacle(_mp(ty, tx)) && euclidean(sx, sy, tx, ty) > 0.5 * max_euclidean);
    jerry_.first = ty;
    jerry_.second = tx;
  }

  void start_and_wait() {
    while (tom_ != jerry_) {
      // ask for the player to play
      VPI path(player_.plan(tom_, jerry_));
      if (path.empty()) { cout << " No path\n"; break; }

      // print the board
      SPI path_set(path.begin(), path.end());
      system("clear");
      for (int i = 0; i < board_.num_rows(); i++) {
        for (int j = 0; j < board_.num_cols(); j++) {
          PI pos(i, j);
          if (pos == tom_) {
            cout << "\033[1;34mT\033[0m";
          } else if (pos == jerry_) {
            cout << "\033[1;32mJ\033[0m";
          } else if (path_set.end() != path_set.find(pos)) {
            cout << "\033[1;31m+\033[0m";
          } else if (player_.explored(pos)) {
            cout << "\033[1;33me\033[0m";
          } else if (board_.obstacle(pos)) {
            cout << "\033[1;30mx\033[0m";
          } else {
            cout << "o";
          }
        }
        cout << "\n";
      }

      tom_ = path[1];
      if (tom_ == jerry_) { cout << "Tom ate Jerry\n"; break; }

      // randomly move jerry to an adjacent cell
      VPI jerry_adj;
      for (int k = -3; k <= 3; k++) {
        for (int l = -3; l <= 3; l++) {
          PI p(jerry_.first + k, jerry_.second + l);
          if (!board_.contains(p)) { continue; }
          if (board_.obstacle(p)) { continue; }
          if (p == tom_) { continue; }
          jerry_adj.push_back(p);
        }
      }
      if (jerry_adj.empty()) { cout << "Jerry in stalemate\n"; break; }

      int jerry_jump = rand() % jerry_adj.size();
      jerry_ = jerry_adj[jerry_jump];

      // give some breathing space (or time??)
      usleep(1000000);
    }
  }

 private:
  Board board_;
  Player player_;
  PI tom_;
  PI jerry_;
};

int main(int argc, char ** argv) {
  srand(time(0));
  int N = rand() % 50;
  int M = rand() % 100 + 50;
  Board board(N, M);
  int (*heuristic_fn)(int, int, int, int) = argc == 1 ? euclidean : naive;
  Player player(board, heuristic_fn);
  Game game(board, player);
  game.start_and_wait();

  return 0;
}
