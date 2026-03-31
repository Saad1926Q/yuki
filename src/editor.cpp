#include "editor.hpp"
#include "libs/chai/search.hpp"
#include <cstddef>
#include <ncurses.h>
#include <string>


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
std::vector<TextRow> EditorState::getTextRows() { return textRows; }
TextRow& EditorState::getTextRow(int idx) { return textRows[idx]; }
std::string EditorState::getFileName() { return filename; }

SearchHistory& EditorState::getSearchHistory(){return searchHistory;}
std::string& EditorState::getCurrentPattern(){return searchHistory.pattern;}

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
    TextRow txtRow;
    txtRow.text = text;
    txtRow.size = text.size();
    numRows += 1;
    textRows.push_back(txtRow);
    dirty = true;
}

void EditorState::insertRow(std::string text, int pos) {
    TextRow newRow;
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
        TextRow &row = textRows[r];
        if (c >= 0 && c < row.size) {
            row.text.insert(c, 1, ch);
        } else if (c == row.size) {
            row.text += ch;
        }
        row.size++;
        dirty = true;
    }
}


void EditorState::updateSearchPattern(std::string str){
    searchHistory.pattern=str;
}

void EditorState::updateSearchHistory(SearchHistory s){
    searchHistory=s;
}

void EditorState::updateSearchDirection(bool isForward){ // Set search direction , true if forward
    searchHistory.forward=isForward;
}


void EditorState::searchPattern(){
    if(searchHistory.forward){
        bool found=false;
        for(std::size_t i=cursorFileY;i<textRows.size();i++){
            std::string rowContent=textRows[i].text;

            if(i==cursorFileY)
                rowContent=rowContent.substr(cursorFileX);

            int idx=chai::search(rowContent,searchHistory.pattern);

            if(idx>=0){

                if(i==cursorFileY)
                    cursorFileX=idx+cursorFileX;
                else
                    cursorFileX=idx;

                cursorFileY=i;
                found=true;
                break;
            }
        }
        if(!found){
            for(std::size_t j=0;j<cursorFileY;j++){
                std::string rowContent=textRows[j].text;

                int idx=chai::search(rowContent,searchHistory.pattern);

                if(idx>=0){

                    if(j==cursorFileY)
                        cursorFileX=idx+cursorFileX;
                    else
                        cursorFileX=idx;

                    cursorFileY=j;
                    break;
                }
            }
        }
    }else{
        bool found=false;
        for(std::size_t j=cursorFileY;j>=0;j--){
            std::string rowContent=textRows[j].text;

            if(j==cursorFileY)
                rowContent=rowContent.substr(cursorFileX);

            int idx=chai::search(rowContent,searchHistory.pattern);

            if(idx>=0){

                if(j==cursorFileY)
                    cursorFileX=idx+cursorFileX;
                else
                    cursorFileX=idx;

                cursorFileY=j;
                found=true;
                break;
            }
        }

        if(!found){
            for(std::size_t i=textRows.size();i>cursorFileY;i--){
                std::string rowContent=textRows[i].text;

                if(i==cursorFileY)
                    rowContent=rowContent.substr(cursorFileX);

                int idx=chai::search(rowContent,searchHistory.pattern);

                if(idx>=0){

                    if(i==cursorFileY)
                        cursorFileX=idx+cursorFileX;
                    else
                        cursorFileX=idx;

                    cursorFileY=i;
                    break;
                }
            }
        }
    }
}

void EditorState::displayStatusBar(StatusBar& S){
    attron(A_REVERSE);

    int terminalCols = getCols();
    int row_cnt = getRows() - 1;

	if (S.leftText.length() > terminalCols) {
        S.leftText.resize(terminalCols);
	}

    mvprintw(row_cnt, 0, "%s", S.leftText.c_str());

	if (S.leftText.length() + S.rightText.length() < terminalCols) {
    	mvprintw(row_cnt, getCols()-S.rightText.size(), "%s", S.rightText.c_str());
	}

    attroff(A_REVERSE);
}

void EditorState::updateStatusBarCoordinates(StatusBar& S,int row,int col){
    S.r=row;
    S.c=col;
}

// the implementations for the reset functions
void EditorState::onFileLoad() {
    dirty = false;
}

void EditorState::onFileSave() {
    dirty = false;
}



void EditorState::fileCoordinatesToScreenCoordinates(){
    if(cursorFileY<rowOffset){
        rowOffset=cursorFileY;
    }

    int textWindowHeight = getTextWindowHeight();

    if(cursorFileY>=rowOffset+textWindowHeight){
        rowOffset=cursorFileY-textWindowHeight+1;
    }

    if(cursorFileX<colOffset){
        colOffset=cursorFileX;
    }

    if(cursorFileX>=colOffset+terminalCols){
   		colOffset=cursorFileX-terminalCols+1;
   	}

    int r = cursorFileY - rowOffset;
   	int c = cursorFileX - colOffset;

    if(r>=textWindowHeight){
        r=textWindowHeight-1;
    }

    cursorX=c;
    cursorY=r;
}
