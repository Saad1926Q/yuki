# yuki

**yuki** is a simple terminal-based text editor written in C++.  
The idea is to build a lightweight editor from scratch, focusing first on core editing features.

![yuki logo](yuki_logo.png)


## Goals

The initial goal is to implement:
- Basic insertion
- Basic deletion
- Basic file operations

It's not aiming to be a full-fledged editor (yet), just something minimal that works.

## Dependencies

- [`ncurses`](https://invisible-island.net/ncurses/): used for handling terminal input and display.

## Build

To build the project, make sure you have `g++` and `ncurses` installed, then:

```bash
g++ main.cpp -lncurses -o yuki
```