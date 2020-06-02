#include <ncurses.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <locale>
#include <vector>

enum Action { NONE, FILLING, CROSSING, EMPTYING_FILL, EMPTYING_CROSS };
const char *DebugAction[] = {"NONE", "FILLING", "CROSSING", "EMPTYING_FILL",
                             "EMPTYING_CROSS"};

enum State { EMPTY, FILLED, CROSSED };
// const char *DebugState[] = {"EMPTY", "FILLED", "CROSSED"};
const char *show[] = {" ", "O", "✕"};
// const char *show[] = {" ", "█", "."};

enum Key {
  K_QUIT = 'q',
  K_UP = 'w',
  K_DOWN = 's',
  K_LEFT = 'a',
  K_RIGHT = 'd',
  K_FILL = 'z',
  K_CROSS = 'x',
  K_CLEAR = 'c',
};

typedef struct {
  int r, c;
  Action action;
  bool repeating;
} Move;

typedef struct {
  Key key;
  bool holding;
} Input;

class Board {
 protected:
  int size_;
  State *board_;

  bool inline in_range(int r, int c) const {
    return 0 <= r && r < size_ &&  //
           0 <= c && c < size_;
  }

 public:
  Board(int s) : size_(s) { board_ = new State[s * s]; }

  inline int size() const { return size_; }
  inline State *grid() const { return board_; }

  State get(int r, int c) const {
    if (in_range(r, c)) {
      return (State)board_[r * size_ + c];
    }

    return EMPTY;
  }

  void set(int r, int c, State s) {
    if (in_range(r, c)) {
      board_[r * size_ + c] = s;
    }
  }
};

class Grid : public Board {
 private:
  static const int space_ = 2;

  Board &solution;

  Move previous_;

  WINDOW *grid_;
  WINDOW *rows_;
  WINDOW *columns_;

  std::vector<std::vector<int>> counts_row;
  std::vector<std::vector<int>> counts_col;
  int max_in_row = 0;
  int max_in_col = 0;

  void count(Board &b) {
    int n;
    for (int r = 0; r < size_; r++) {
      std::vector<int> row;
      n = 0;

      for (int c = 0; c < size_; c++) {
        if (b.get(r, c) == FILLED) {
          n++;
        } else if (n > 0) {  // end of block
          row.push_back(n);
          n = 0;
        }
      }
      if (n > 0) {  // check again at end of line
        row.push_back(n);
        n = 0;
      }

      max_in_row = std::max(max_in_row, (int)row.size());
      counts_row.push_back(row);
    }

    for (int c = 0; c < size_; c++) {
      std::vector<int> col;
      n = 0;

      for (int r = 0; r < size_; r++) {
        if (b.get(r, c) == FILLED) {
          n++;
        } else if (n > 0) {  // end of block
          col.push_back(n);
          n = 0;
        }
      }
      if (n > 0) {  // check again at end of line
        col.push_back(n);
        n = 0;
      }

      max_in_col = std::max(max_in_col, (int)col.size());
      counts_col.push_back(col);
    }
  }

 public:
  Grid(Board &s) : Board(s.size()), solution(s) {
    count(s);

    grid_ =
        newwin(size_ + 2, size_ * space_ + 1, max_in_col, 2 * max_in_row + 1);
    rows_ = newwin(size_ + 2, 2 * max_in_row + 2, max_in_col, 0);
    columns_ =
        newwin(max_in_col + 2, size_ * space_ + 1, 0, 2 * max_in_row + 1);

    keypad(grid_, true);

    for (int n = 0; n < size_; n++) {
      checkCompleted(n, 0);
      checkCompleted(0, n);
    }

    previous_ = {0, 0, NONE, false};
  }

  inline int input() { return wgetch(grid_); }

  void checkCompleted(int r_, int c_) {
    std::vector<int> cur_row;
    std::vector<int> cur_col;

    int n = 0;
    for (int c = 0; c < size_; c++) {
      if (get(r_, c) == FILLED) {
        n++;
      } else if (n > 0) {
        cur_row.push_back(n);
        n = 0;
      }
    }
    if (n > 0) {
      cur_row.push_back(n);
      n = 0;
    }
    for (int r = 0; r < size_; r++) {
      if (get(r, c_) == FILLED) {
        n++;
      } else if (n > 0) {
        cur_col.push_back(n);
        n = 0;
      }
    }
    if (n > 0) {
      cur_col.push_back(n);
      n = 0;
    }

    if (cur_row == counts_row[r_]) {
      for (int c = 0; c < size_; c++) {
        if (get(r_, c) != FILLED) {
          set(r_, c, CROSSED);
        }
      }
    }
    if (cur_col == counts_col[c_]) {
      for (int r = 0; r < size_; r++) {
        if (get(r, c_) != FILLED) {
          set(r, c_, CROSSED);
        }
      }
    }
  }

  void update(Input in) {
    int r = previous_.r;
    int c = previous_.c;
    Action action = NONE;
    bool repeating = in.holding;

    if (repeating) {
      action = previous_.action;
    }

    // handle navigation

    if (in.key == K_UP) {
      r -= 1;
    } else if (in.key == K_DOWN) {
      r += 1;
    } else if (in.key == K_LEFT) {
      c -= 1;
    } else if (in.key == K_RIGHT) {
      c += 1;
    }

    r = (r + size_) % size_;
    c = (c + size_) % size_;

    State state = get(r, c);

    // determine action

    if (in.key == K_FILL) {
      if (state == FILLED) {
        action = EMPTYING_FILL;
      } else {
        action = FILLING;
      }
    }

    if (in.key == K_CROSS) {
      if (state == CROSSED) {
        action = EMPTYING_CROSS;
      } else {
        action = CROSSING;
      }
    }

    // determine new state

    if (action == FILLING) {
      if (state == EMPTY) {
        state = FILLED;
      }
      if (!repeating && state == CROSSED) {
        state = EMPTY;
      }
    }
    if (action == CROSSING) {
      if (state == EMPTY) {
        state = CROSSED;
      }
      if (!repeating && state == FILLED) {
        state = EMPTY;
      }
    }
    if ((action == EMPTYING_FILL && state == FILLED) ||
        (action == EMPTYING_CROSS && state == CROSSED)) {
      state = EMPTY;
    }

    if (in.key == K_CLEAR) {
      if (!in.holding) {
        move(0, 30);
        printw("Press SHIFT + %c to confirm clearing", K_CLEAR);
      } else {
        for (int r = 0; r < size_; r++) {
          for (int c = 0; c < size_; c++) {
            set(r, c, EMPTY);
          }
        }
        move(0, 30);
        clrtoeol();
      }
    } else {
      move(0, 30);
      clrtoeol();
    }

    set(r, c, state);

    // if we are filling and not emptying
    if (action == FILLING && state != EMPTY) {
      checkCompleted(r, c);
    }

    previous_ = {r, c, action, repeating};
  }

  void draw() {
    box(grid_, 0, 0);
    for (int r = 0; r < size_; r++) {
      for (int c = 0; c < size_; c++) {
        wmove(grid_, r + 1, c * space_ + 1);
        wprintw(grid_, show[get(r, c)]);
      }
    }

    for (int r = 0; r < size_; r++) {
      std::vector<int> row = counts_row[r];
      wmove(rows_, r + 1, (max_in_row - row.size()) * 2);
      for (int n = 0; n < row.size(); n++) {
        wprintw(rows_, "%2d", row[n]);
      }
    }
    for (int c = 0; c < size(); c++) {
      std::vector<int> col = counts_col[c];
      for (int n = 0; n < col.size(); n++) {
        wmove(columns_, max_in_col - col.size() + n, c * space_);
        wprintw(columns_, "%*d", space_, col[n]);
      }
    }

    refresh();
    wrefresh(rows_);
    wrefresh(columns_);
    wrefresh(grid_);
  }

  void highlight() {
    int r_ = previous_.r;
    int c_ = previous_.c;

    for (int r = 0; r < size_; r++) {
      mvwchgat(rows_, r + 1, 0, max_in_row * 2 + 1, A_NORMAL, 0, NULL);
      mvwchgat(grid_, r + 1, 1, size_ * space_ - 1, A_NORMAL, 0, NULL);
    }

    for (int r = 0; r < max_in_col; r++) {
      mvwchgat(columns_, r, 0, size_ * space_, A_NORMAL, 0, NULL);
    }

    mvwchgat(rows_, r_ + 1, 0, max_in_row * 2 + 1, A_STANDOUT, 1, NULL);
    for (int r = 0; r < max_in_col; r++) {
      mvwchgat(columns_, r, c_ * space_ + 1, 1, A_STANDOUT, 1, NULL);
    }
    for (int r = 0; r < size_; r++) {
      if (r == r_) {
        mvwchgat(grid_, r + 1, 1, size_ * space_ - 1, A_STANDOUT, 1, NULL);
      } else {
        mvwchgat(grid_, r + 1, c_ * space_ + 1, 1, A_STANDOUT, 1, NULL);
      }
    }

    mvwchgat(grid_, r_ + 1, c_ * space_ + 1, 1, A_NORMAL, 1, NULL);

    wrefresh(rows_);
    wrefresh(columns_);
    wrefresh(grid_);

    wmove(grid_, r_ + 1, c_ * space_ + 1);
  }

  bool solved() {
    for (int r = 0; r < size_; r++) {
      for (int c = 0; c < size_; c++) {
        if ((solution.get(r, c) == FILLED && get(r, c) != FILLED) ||
            (solution.get(r, c) != FILLED && get(r, c) == FILLED)) {
          return false;
        }
      }
    }

    return true;
  }
};

Input handle(int ch) {
  Key key = K_QUIT;
  bool holding = false;

  if (ch == KEY_SR || ch == toupper(K_UP) ||         //
      ch == KEY_SF || ch == toupper(K_DOWN) ||       //
      ch == KEY_SLEFT || ch == toupper(K_LEFT) ||    //
      ch == KEY_SRIGHT || ch == toupper(K_RIGHT) ||  //
      ch == toupper(K_CLEAR)) {
    holding = true;
  }

  if (ch == KEY_SR || ch == KEY_UP || ch == toupper(K_UP)) {
    key = K_UP;
  } else if (ch == KEY_SF || ch == KEY_DOWN || ch == toupper(K_DOWN)) {
    key = K_DOWN;
  } else if (ch == KEY_SLEFT || ch == KEY_LEFT || ch == toupper(K_LEFT)) {
    key = K_LEFT;
  } else if (ch == KEY_SRIGHT || ch == KEY_RIGHT || ch == toupper(K_RIGHT)) {
    key = K_RIGHT;
  } else if (ch == toupper(K_CLEAR)) {
    key = K_CLEAR;
  }

  if (ch == K_FILL) {
    key = K_FILL;
  }

  if (ch == K_CROSS) {
    key = K_CROSS;
  }

  if (ch == K_CLEAR) {
    key = K_CLEAR;
  }

  return {key, holding};
}

int main(int argc, char **argv) {
  setlocale(LC_ALL, "");

  if (argc != 2) {
    std::cerr << "Usage: nono [puzzles file]" << std::endl;
    exit(-1);
  }

  std::ifstream puzzles(argv[1]);
  if (!puzzles.good()) {
    std::cerr << "File '" << argv[1] << "' could not be found" << std::endl;
    exit(-1);
  }

  char *name = (char *)malloc(256);
  int size = 0;

  puzzles >> name;
  puzzles >> size;

  Board solution(size);

  char n = 0;
  int s = 0;

  for (int r = 0; r < size; r++) {
    for (int c = 0; c < size; c++) {
      puzzles >> n;
      s = n - '0';

      if (s == 0) {
        solution.set(r, c, EMPTY);
      } else if (s == 1) {
        solution.set(r, c, FILLED);
      } else {
        std::cerr << "Puzzle '" << name << "' has an invalid bit - " << n
                  << std::endl;
        exit(-1);
      }
    }
  }

  initscr();
  cbreak();
  noecho();
  keypad(stdscr, true);
  start_color();
  use_default_colors();
  init_pair(1, COLOR_YELLOW, COLOR_BLACK);
  curs_set(0);

  const int height = getmaxy(stdscr);
  const int width = getmaxx(stdscr);

  Grid board(solution);

  move(0, 0);
  printw("%s", name);

  // add redrawing with new window size??

  move(height - 2, 0);
  printw("Fill - %c, Cross Out - %c, Quit - %c", K_FILL, K_CROSS, K_QUIT);
  move(height - 1, 0);
  printw("Hold Shift to repeat last action");

  board.draw();
  board.highlight();

  int ch;
  while ((ch = board.input())) {
    if (ch == K_QUIT) {
      break;
    }

    board.update(handle(ch));

    // move(height - 4, 0);
    // clrtoeol();
    // printw("%3d %3d %d %s", previous.r, previous.c, previous.repeating,
    //        DebugAction[previous.action]);
    move(height - 3, 0);
    printw("Solved: %d", board.solved());

    board.draw();
    board.highlight();
  }

  endwin();
  return 0;
}