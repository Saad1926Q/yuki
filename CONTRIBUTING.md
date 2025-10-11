# Contributing to Yuki

This project is a personal journey into understanding how text editors work at a fundamental level. It's built for the sheer love of the game, for those who find joy in the intricate details of crafting software from the ground up.

**This is not a project for those chasing random shiny badges like Hacktoberfest or superficial contributions.**

**Please, don't mess up the codebase with vibe-coded slop. Keep your AI-generated slop away from this codebase.** 

If you're passionate about low-level programming and text editors, your contributions are genuinely welcome.

---

## Codebase Overview

Yuki follows a clear separation of concerns, primarily influenced by the Model-View-Controller (MVC) pattern, although simplified for a terminal application.

### `EditorState` Class

`EditorState` is the single source of truth for all data related to the open file and the editor's current context.

* **`textRows`**: A `std::vector` of `textRow` structs. This vector holds the actual content of the file, line by line.
* **`numRows`**: The total number of lines in the file.
* **`cursorFileY` & `cursorFileX`**: These represent the **logical cursor position** within the file's data (`textRows`). `cursorFileY` is the row index, and `cursorFileX` is the column index within that specific row's string. These are the *true* coordinates of the cursor in the document, regardless of scrolling.
* **`rowOffset` & `colOffset`**: These are the **scrolling offsets**.
    * `rowOffset`: The index of the file line that is currently displayed at the very top of the screen. This determines vertical scrolling.
    * `colOffset`: The character index within a line that is currently displayed at the very left edge of the screen. This determines horizontal scrolling.
* **`cursorY` & `cursorX`**: These represent the **physical cursor position** on the terminal screen. These are the `(y, x)` coordinates used by ncurses' `move()` function. They are *derived* from `cursorFileY`, `cursorFileX`, and the offsets.

### `textRow` Struct

This simple struct represents a single line of text in the file.

* **`size`**: The length of the `text` string.
* **`text`**: A `std::string` containing the actual characters of the line.

### `AppendBuffer` Class

This is primarily a helper class for efficient screen rendering.

* It acts as a temporary buffer to build the entire screen content (all visible lines) as a single string before printing it to the ncurses window in one go. This minimizes calls to ncurses' `printw()` and improves performance by reducing flickering.
* **How it's updated each cycle:** In the `refreshScreen()` function, the `AppendBuffer` is cleared, then each visible line from `EditorState::textRows` (adjusted by `rowOffset` and `colOffset`) is formatted and appended to it. Finally, the entire buffer content is printed to the screen.

1.  **Input (`editorProcessKeypress()`):**
    * Reads a single keypress from the user.
    * Based on the key, it *only* updates the **logical cursor position** (`cursorFileY`, `cursorFileX`) and calls methods on `EditorState` to modify the file data (`textRows`).
    * It *does not* directly change `r`, `c`, `rowOffset`, or `colOffset` here, keeping the focus purely on the model.

    * After the logical cursor has potentially moved, this section checks if `cursorFileY` or `cursorFileX` are now outside the currently visible viewport (defined by `rowOffset`, `colOffset`, `terminalRows`, `terminalCols`).
    * If the cursor is off-screen, `rowOffset` or `colOffset` are adjusted to bring it back into view.

2.  **Screen Update (`refreshScreen()`):**
    * This function's sole responsibility is to render the current state of `EditorState` onto the terminal.
    * It uses `rowOffset` and `colOffset` to determine which part of the `textRows` data should be drawn.
    * The final screen cursor positions (`E.cursorY`, `E.cursorX`) are *calculated* from `E.cursorFileY`, `E.cursorFileX`, `rowOffset`, and `colOffset` right before `move(r,c)` is called.


---

## How to Contribute

Contributions are welcome from individuals who genuinely want to learn and improve this editor.

* **Fork the Repository:** Start by forking the project to your GitHub account.
* **Create a New Branch:** Always work on a new branch for your feature or bug fix.
* **Code Style:**
    * Please stick to `camelCase` for variables and functions.
    * Use `PascalCase` for class names.
    * Place opening curly braces `{` on a new line.
* **Documentation:** Add comments for complex logic, new functions, or any non-obvious code. Explain the *why*, not just the *what*.
* **Testing:** Test your changes thoroughly.
* **Pull Requests:** When submitting a pull request, provide a clear description of what you've changed and why.

If you have questions or want to discuss a feature, feel free to open an issue!