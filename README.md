<h1 align="center">yuki</h1>

<p align="center">
A simple terminal-based text editor written in C++
</p>

<div align="center">
<img width="70%" alt="yuki logo" src="yuki_logo.png" />
</div>

---

## Goals

The initial goal is to implement a robust, minimal text editor.

## Features Implemented

- **File Operations:** Open existing text files and save changes (Ctrl+S).
- **Text Insertion:** Add characters at any point in a line.
- **Backspace:** Delete characters (before the cursor) and merge lines if at the beginning of a line.
- **Enter Key:** Split a line at the cursor's position, moving subsequent text to a new line.
- **Cursor Movement:** Full navigation with arrow keys (Up, Down, Left, Right), Home (jumps to the start of the line), and End (jumps to the end of the line). Includes intelligent boundary checking and cursor "snapping".
- **Modes:** Vim-like normal and insert modes.
- **Vim Motions:** Basic vim motions including G (jump to end of file) and gg (jump to start of file).
- **Search:** Basic string search functionality.
- **Status Bar:** Displays filename, cursor position, and mode information.
- **Exit:** Quit the editor (F1).

## How to Use Yuki

### Build

Make sure you have `g++` and the `ncurses` library installed.

```bash
g++ main.cpp editor.cpp libs/chai/search.cpp -o yuki -lncurses
```

**For contributors:** Make sure to add the [chai](https://github.com/Saad1926Q/chai) library as a submodule.

### Run

```bash
./yuki <filename>
```

Example: `./yuki my_notes.txt`

## Controls

- **Arrow Keys** - Move the cursor
- **Printable Characters** - Type text
- **Enter** - Split the current line and move the cursor to the new line
- **Backspace** - Delete the character before the cursor or merge the current line with the previous one
- **Ctrl+S** - Save the current file
- **F1** - Exit the editor

## Dependencies

- [`ncurses`](https://invisible-island.net/ncurses/) - Used for handling terminal input and display
- [`chai`](https://github.com/Saad1926Q/chai) - Custom library for string search and utilities

## Known Issues

- **Search Edge Cases:** Search functionality does not work properly when there are blank lines before the target pattern.
- **Rendering Issues:** Unnecessary blank lines are sometimes added during rendering.
- **Cursor Navigation:** In some files (not all), the cursor cannot move below a certain point even though more content exists below.

## TODOs

- [x] Add a status bar showing filename, cursor row/column, and save status.
- [x] Implement modes (similar to Vim).
- [x] Add Vim Motions (basic motions like G and gg implemented).
- [ ] Add support for custom key bindings
- [ ] Add support for handling complex characters and encoding beyond basic ASCII.
- [ ] Add a "dirty" flag to prompt user to save before quitting(currently it always auto-saves on quit)

## Inspirations & Motivation

I'm not trying to reinvent the wheel or claim to replace any existing text editors.
I'm just building this out of pure curiosity - doing it for the love of the game.

> Note: As this is my first C++ project, the code might not be the most optimized, and there's a possibility of bugs.

- **Inspired by:** Some parts of the code were inspired by the excellent [Kilo editor tutorial](https://viewsourcecode.org/snaptoken/kilo/).
- **Main Motivation:** The idea for building a text editor from scratch was largely motivated by this video: [How to build a text editor (YouTube)](https://www.youtube.com/watch?v=g2hiVp6oPZc).

<br><br>
