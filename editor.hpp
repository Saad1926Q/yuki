#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <string>
#include <vector>

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
    std::vector<textRow> getTextRows();
    textRow& getTextRow(int idx);
    std::string getFileName();

    void setRows(int rows);
    void setCols(int cols);
    void setRowOffset(int val);
    void setColOffset(int val);
    void setCursorX(int val);
    void setCursorY(int val);
    void setCursorFileY(int val);
    void setCursorFileX(int val);
    void setFileName(std::string str);

    int getWindowSize();
    void appendRow(std::string text);
    void insertRow(std::string text, int pos);
    void removeRow(int pos);
    void editorInsertChar(char ch, int r, int c);
};

#endif 