# yuki

**yuki** is a simple terminal-based text editor written in C++. The idea is to build a lightweight editor from scratch, focusing first on core editing features.

![yuki logo](yuki_logo.png)

## Goals

The initial goal is to implement a robust, minimal text editor.

## Features Implemented

* **File Operations:** Open existing text files and save changes (Ctrl+S).
* **Text Insertion:** Add characters at any point in a line.
* **Backspace:** Delete characters (before the cursor) and merge lines if at the beginning of a line.
* **Enter Key:** Split a line at the cursor's position, moving subsequent text to a new line.
* **Cursor Movement:** Full navigation with arrow keys (Up, Down, Left, Right), Home (jumps to the start of the line), and End (jumps to the end of the line). Includes intelligent boundary checking and cursor "snapping".
* **Exit:** Quit the editor (F1).

## How to Use Yuki

1.  **Build:** Make sure you have `g++` and the `ncurses` library installed.
    ```bash
    g++ main.cpp editor.cpp -lncurses -o yuki
    ```
2.  **Run:**
        ```bash
        ./yuki <filename>
        ```
        Example: `./yuki my_notes.txt`

3.  **Basic Controls:**
    * **Arrow Keys:** Move the cursor.
    * **Printable Characters:** Type text.
    * **Enter:** Split the current line and move the cursor to the new line.
    * **Backspace:** Delete the character before the cursor or merge the current line with the previous one.
    * **Ctrl+S:** Save the current file.
    * **F1:** Exit the editor.

## Inspirations & Motivation

I'm not trying to reinvent the wheel or claim to replace any existing text editors.
I'm just building this out of pure curiosity - doing it for the love of the game.

> Note: As this is my first C++ project, the code might not be the most optimized, and there's a possibility of bugs.

* **Inspired by:** Some parts of the code were inspired by the excellent [Kilo editor tutorial](https://viewsourcecode.org/snaptoken/kilo/).
* **Main Motivation:** The idea for building a text editor from scratch was largely motivated by this video: [How to build a text editor (YouTube)](https://www.youtube.com/watch?v=g2hiVp6oPZc).

## Dependencies

* [`ncurses`](https://invisible-island.net/ncurses/): Used for handling terminal input and display.

## TODOs

* Add a status bar showing filename, cursor row/column, and save status.
* Implement modes (similar to Vim).
* Add Vim Motions.
* Add support for handling complex characters and encoding beyond basic ASCII.
* Add a "dirty" flag to prompt user to save before quitting(currently it always auto-saves on quit)