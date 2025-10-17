#include "editor.hpp"
#include <ncurses.h>

std::string AppendBuffer::getBuffer() {
    return buffer;
}

void AppendBuffer::append(const std::string& str) {
    buffer += str;
}

void AppendBuffer::clear() {
    buffer.clear();
}

EditorState::EditorState() {
    numRows = 0;
    rowOffset = 0;
    colOffset = 0;
    cursorX = 0;
    cursorY = 0;
    cursorFileY = 0;
    dirty = false;
    // set the mode to Normal by default
    currentMode = EditorMode::Normal;
}

// the implementation of the isDirty function
bool EditorState::isDirty() const {
    return dirty;
}
void EditorState::markAsDirty() {
    dirty = true;
}

int EditorState::getRows() { return terminalRows; }
int EditorState::getCols() { return terminalCols; }
int EditorState::getNumRows() { return numRows; }
int EditorState::getRowOffset() { return rowOffset; }
int EditorState::getColOffset() { return colOffset; }
int EditorState::getCursorX() { return cursorX; }
int EditorState::getCursorY() { return cursorY; }
int EditorState::getCursorFileY() { return cursorFileY; }
int EditorState::getCursorFileX() { return cursorFileX; }
std::vector<textRow> EditorState::getTextRows() { return textRows; }
textRow& EditorState::getTextRow(int idx) { return textRows[idx]; }
std::string EditorState::getFileName() { return filename; }
//Implement the getter for the mode
EditorMode EditorState::getMode() const {
    return currentMode;
}

int EditorState::getTextWindowHeight(){
    // The text window is the total height minus the status bar
    return terminalRows - 1;
}

void EditorState::setRows(int rows) { terminalRows = rows; }
void EditorState::setCols(int cols) { terminalCols = cols; }
void EditorState::setRowOffset(int val) { rowOffset = val; }
void EditorState::setColOffset(int val) { colOffset = val; }
void EditorState::setCursorX(int val) { cursorX = val; }
void EditorState::setCursorY(int val) { cursorY = val; }
void EditorState::setCursorFileY(int val) { cursorFileY = val; }
void EditorState::setCursorFileX(int val) { cursorFileX = val; }
void EditorState::setFileName(std::string str) { filename = str; }
//Implement the setter for the mode
void EditorState::setMode(EditorMode mode) {
    currentMode = mode;
}

int EditorState::getWindowSize() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    if (rows == 0 || cols == 0)
        return -1;
    setRows(rows);
    setCols(cols);
    return 0;
}


// set dirty => true in all functions that change text

void EditorState::appendRow(std::string text) {
    textRow txtRow;
    txtRow.text = text;
    txtRow.size = text.size();
    numRows += 1;
    textRows.push_back(txtRow);
    dirty = true; 
}

void EditorState::insertRow(std::string text, int pos) {
    textRow newRow;
    newRow.text = text;
    newRow.size = text.size();
    textRows.insert(textRows.begin() + pos, newRow);
    numRows++;
    dirty = true; 
}

void EditorState::removeRow(int pos) {
    textRows.erase(textRows.begin() + pos);
    numRows--;
    dirty = true; 
}

void EditorState::editorInsertChar(char ch, int r, int c) {
    int numFileRows = getNumRows();
    if (r >= 0 && r < numFileRows) {
        textRow &row = textRows[r];
        if (c >= 0 && c < row.size) {
            row.text.insert(c, 1, ch);
        } else if (c == row.size) {
            row.text += ch;
        }
        row.size++;
        dirty = true; 
    }
}

// the implementations for the reset functions
void EditorState::onFileLoad() {
    dirty = false;
}

void EditorState::onFileSave() {
    dirty = false;
}