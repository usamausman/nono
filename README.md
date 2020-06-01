# nonogram

## What is this?

`nonogram` is a implementation of [nonogram puzzles](https://en.wikipedia.org/wiki/Nonogram) which can be played in a terminal window

## Requirements

* CMake 3.1+
* Support for C++ 11

## Building

```terminal
cd build
cmake ..
make
```

## Running

```terminal
./build/nono puzzles.txt
```

Currently, `nonogram` will load the first puzzle defined in `puzzles.txt`. These puzzle format is defined as such:

```
name
n
n by n grid of cells, where 0 is empty and 1 is filled
```

## Instructions

- Arrow keys or <kbd>WASD</kbd> - move around the board
- <kbd>Z</kbd> - fill the current cell
- <kbd>X</kbd> - cross out the current cell
- Note: Holding <kbd>Shift</kbd> will repeat the previous action
- <kbd>Q</kbd> - quit

## TODO

- [ ] Move to standard nonogram format
- [ ] Add menu to choose a puzzle from `puzzles.txt`
- [ ] Add dialog when puzzle is completed
