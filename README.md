# nono

## What is this?

`nono` is a implementation of [nonogram puzzles](https://en.wikipedia.org/wiki/Nonogram) which can be played in a terminal window

## Requirements

* CMake 3.1+
* C++11 compiler
* `ncurses` with wide character support

You can install `ncurses` from your favourite package manager (`apt`, `yum`, `brew`, etc.)

## Building & Running

```terminal
./make.sh
./build/nono puzzles.txt
```

**Note:** Currently, `nono` will only read the first puzzle from `puzzles.txt`.

## Controls

* Arrow keys or <kbd>WASD</kbd> - move around the board
* <kbd>Shift + C</kbd> - clear the board
* <kbd>Q</kbd> - quit

### Actions

Holding <kbd>Shift</kbd> will repeat the previous action

* <kbd>Z</kbd> - fill the current cell
* <kbd>X</kbd> - cross out the current cell

## Puzzle Format

```
name
n
n by n grid of cells, where 0 is empty and 1 is filled
```

## TODO

- [ ] Fix highlighting colours
- [ ] Move to standard nonogram format
- [ ] Add menu to choose a puzzle from `puzzles.txt`
- [ ] Add dialog when puzzle is completed
