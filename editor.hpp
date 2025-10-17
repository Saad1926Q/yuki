#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <string>
#include <vector>

// define the editor modes here
enum class EditorMode {
    Normal,
    Insert
};

struct textRow {
    int size;
    std::string text;
};

class AppendBuffer {
    std::string buffer;
public:
    std::string getBuffer();
    void append(const std::string& str);
    void clear();
};

class EditorState {
    int terminalRows;
    int terminalCols;
    int numRows;
    int rowOffset;
    int colOffset;
    int cursorX;
    int cursorY;
    int cursorFileY;
    int cursorFileX;
    std::string filename;
    std::vector<textRow> textRows;

    bool dirty;
    // the current mode is now part of the editor's state
    EditorMode currentMode;

public:
    EditorState();
    int getRows();
    int getCols();
    int getNumRows();
    int getRowOffset();
    int getColOffset();
    int getCursorX();
    int getCursorY();
    int getCursorFileY();
    int getCursorFileX();
    int getTextWindowHeight();   // This is after considering that the last row is reserved for status bar
    std::vector<textRow> getTextRows();
    textRow& getTextRow(int idx);
    std::string getFileName();
 EditorMode getMode() const; // get cur mode

    // public getter for dirty flag
    bool isDirty() const;

    void setRows(int rows);
    void setCols(int cols);
    void setRowOffset(int val);
    void setColOffset(int val);
    void setCursorX(int val);
    void setCursorY(int val);
    void setCursorFileY(int val);
    void setCursorFileX(int val);
    void setFileName(std::string str);
    void setMode(EditorMode mode); // set cur mode

    int getWindowSize();
    void appendRow(std::string text);
    void insertRow(std::string text, int pos);
    void removeRow(int pos);
    void editorInsertChar(char ch, int r, int c);

    // add this functions to Manages the dirty flag state
    void markAsDirty();// to set the dirty flag to true
    void onFileLoad();// resets the dirty flag when a file is loaded
    void onFileSave();// resets the dirty flag after a file is saveed
};

#endif 