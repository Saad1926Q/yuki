#include <cctype>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <ncurses.h>
#include <string>
#include "editor.hpp"

AppendBuffer aBuf;
EditorState E;
StatusBar statusBar;


void die(std::string errorMessage){  // For error handling
	endwin();
	std::cerr<<errorMessage<<std::endl;
	std::exit(EXIT_FAILURE);
}


void initEditor(){
	if(E.getWindowSize()==-1)
		die("getWindowSize");
}


void drawContent(){
	int r=E.getCursorY();
	int c=E.getCursorX();
	printw("%s",aBuf.getBuffer().c_str());
	move(r,c);
	aBuf.clear();
}

void appendLineToBuffer(std::string& rowContent,int colOffset=0){
    std::string bufferContent;

    if (colOffset < static_cast<int>(rowContent.size())) {
        std::string rowSubstr = rowContent.substr(colOffset);
        if (static_cast<int>(rowSubstr.size()) > COLS)
            rowSubstr = rowSubstr.substr(0, COLS);

        for (char ch : rowSubstr) {
            if (ch == '\t') {
                for (std::size_t i = 0; i < 4; ++i)
					bufferContent+=" ";
            } else  {
				bufferContent+=ch;
			}
        }
    }

	bufferContent+="\n";
	aBuf.append(bufferContent);
}


void handleFile(char* argv[]){  // Handle the file in the beginning when you run the text editor
    std::string filename{argv[1]};

    std::ifstream file{filename};

    E.setFileName(filename);

    if(!file){
        std::cerr<<"file couldnt be read!!\n";
        std::exit(EXIT_FAILURE);
    }

    std::string currentLine{};

    while(std::getline(file,currentLine)){
        E.appendRow(currentLine);
        appendLineToBuffer(currentLine);
    }

	if (E.getNumRows() == 0)
	    E.appendRow("");

	E.onFileLoad();

	drawContent();
}

void updateFile(){  // Save the content in the file
	std::string filename=E.getFileName();

	std::ofstream file(filename);

	std::vector<TextRow> textRows=E.getTextRows();

	for(std::size_t i=0;i<textRows.size();i++){
		file<<textRows[i].text;

		if(i!=textRows.size()-1)
			file<<"\n";
	}

	file.close();
	E.onFileSave(); // to reset the dirty flag after saving the file
}

void refreshScreen() {
    E.getWindowSize();

    curs_set(0);

    erase();
    aBuf.clear();

    int colOffset=E.getColOffset();
    std::vector<TextRow> textRows=E.getTextRows();

    int rowOffset = E.getRowOffset();
    int terminalRows = E.getRows();
    int maxVisible = std::max(0, terminalRows - 1); // only render visible rows (reserve the last row for status)
    int printed_cnt = 0;

    for(std::size_t i = rowOffset; i < textRows.size() && printed_cnt < maxVisible; ++i){
        std::string rowContent=textRows[i].text;
        appendLineToBuffer(rowContent,colOffset);
        printed_cnt++;
    }

    drawContent();

    EditorMode mode = E.getMode();
    std::string modeString = (mode == EditorMode::Normal) ? "=== NORMAL ===" : "=== INSERT ===";
    std::string name = E.getFileName().substr(0, 20);
    std::string dirtystr = E.isDirty() ? " [Modified , don't forget to save]" : "";

    std::string leftStatus;

    if(mode!=EditorMode::Search)
        leftStatus = modeString + " | " + name + dirtystr;
    else{
        if(E.getSearchHistory().forward)
            leftStatus="\\";
        else
            leftStatus="?";
        leftStatus+=E.getCurrentPattern();
    }

    std::string rightStatus = std::to_string(E.getCursorFileY() + 1) + "/" + std::to_string(E.getNumRows());

    statusBar.rightText=rightStatus;
    statusBar.leftText=leftStatus;

    E.displayStatusBar(statusBar);

    static EditorMode lastMode = EditorMode::Normal;

    if (mode != lastMode) {
        switch (mode) {
            case EditorMode::Normal: // change cursor to a steady block
                printf("\x1b[2 q");
                break;
            case EditorMode::Search:
            case EditorMode::Insert: // change cursor to a blinking bar
                printf("\x1b[5 q");
                break;
        }
        fflush(stdout); // apply the cursor change
        lastMode = mode;
    }

    curs_set(1);

    if(mode==EditorMode::Insert || mode==EditorMode::Normal)
        move(E.getCursorY(), E.getCursorX());
    else{
        E.updateStatusBarCoordinates(statusBar,E.getRows()-1,1+E.getCurrentPattern().size());
        move(statusBar.r,statusBar.c);
    }

    refresh();
}


void editorProcessKeypress() {  // Main function which handles the different key press events
	int ch=getch();


	// Handle Mode Switching First
    if (E.getMode() == EditorMode::Normal && ch == 'i') {
        E.setMode(EditorMode::Insert);
        return; // move to Insert mode and wait for next input
    }

    if (E.getMode() == EditorMode::Normal && ch == '\\') {
        E.setMode(EditorMode::Search);
        E.updateSearchDirection(true); //Search in forward direction
        E.updateSearchPattern("");
        return;
    }
    if (E.getMode() == EditorMode::Normal && ch == '?') {
        E.setMode(EditorMode::Search);
        E.updateSearchDirection(false); //Search in backward direction
        E.updateSearchPattern("");
        return; // move to Insert mode and wait for next input
    }
    if (E.getMode() == EditorMode::Insert && ch == 27) {
        E.setMode(EditorMode::Normal);
        return; // move to normal mode and wait for next input
    }
    if(E.getMode()== EditorMode::Search && ch==27){
        E.setMode(EditorMode::Normal);
        return;
    }

	int rowOffset=E.getRowOffset();
	int colOffset=E.getColOffset();

	int terminalRows=E.getRows();
	int terminalCols=E.getCols();

	int currCursorFileY=E.getCursorFileY(); // Currently which line of the file is the cursor on.
	int currCursorFileX=E.getCursorFileX(); // Currently which col of the file is the cursor on.

	int currRowSize = 0;
	if (E.getNumRows() > 0) {
		currRowSize = E.getTextRow(currCursorFileY).size;
	}

	if (E.getMode() == EditorMode::Normal) {
        switch (ch) {
            case 'k':
				ch = KEY_UP;
				break;
            case 'j':
				ch = KEY_DOWN;
				break;
            case 'h':
				ch = KEY_LEFT;
				break;
            case 'l':
				ch = KEY_RIGHT;
				break;
			case 'G':
				E.setCursorFileY(E.getNumRows()-1);
				E.setCursorFileX(0);
				break;
			case 'g':{
				timeout(300);
				int doubleClick = getch();
				timeout(-1);

				if (doubleClick == 'g')
				{
					E.setCursorFileY(0);
					E.setCursorFileX(0);
				} else if (doubleClick != ERR) {
					ungetch(doubleClick);
				}
				break;
			}

        }
    }

    EditorMode mode = E.getMode();
	switch(ch){
		case KEY_UP:
			if(currCursorFileY!=0){
				E.setCursorFileY(currCursorFileY-1); // Move one line up

				TextRow& newRow = E.getTextRow(currCursorFileY-1);

				if(currCursorFileX>newRow.size)
					E.setCursorFileX(newRow.size); // If the length of new line is smaller than the previous line then snap to the end of the new line

			}
			break;

		case KEY_DOWN:
			if(currCursorFileY<E.getNumRows()-1){
				E.setCursorFileY(currCursorFileY+1); // Move one line down

				TextRow& newRow = E.getTextRow(E.getCursorFileY());

				if(currCursorFileX>newRow.size)
					E.setCursorFileX(newRow.size); // If the length of new line is smaller than the previous line then snap to the end of the new line

			}

			break;

		case KEY_RIGHT:
			if(currCursorFileX!=currRowSize)
				E.setCursorFileX(currCursorFileX+1);


			break;

		case KEY_LEFT:
			if(currCursorFileX!=0)
				E.setCursorFileX(currCursorFileX-1);


			break;

		case KEY_HOME:
			E.setCursorFileX(0);

			break;

		case KEY_END:
			E.setCursorFileX(currRowSize);

			break;

		case KEY_F(1):
			endwin();
			updateFile();
			std::exit(0);
			break;

		case '\n': // Apparently ENTER key is represented by newline character and not by KEY_ENTER
		case KEY_ENTER: // Keypad Enter
			if (mode == EditorMode::Insert) {
				if(E.getNumRows()>0){
					TextRow& currRow=E.getTextRow(currCursorFileY);
					std::string newLineText=currRow.text.substr(currCursorFileX);  // This is the text that will become the next line

					currRow.text=currRow.text.substr(0,currCursorFileX);  // Shorten the original line
					currRow.size = currRow.text.size(); // Update the size of the curr row

					E.insertRow(newLineText,currCursorFileY+1);

					E.setCursorFileX(0); // Move cursor to the beginning of the new line
					E.setCursorFileY(currCursorFileY+1);

				}
			} else if (mode == EditorMode::Search){
			    E.searchPattern();
				E.fileCoordinatesToScreenCoordinates();
				E.setMode(EditorMode::Normal);
			}
			break;

		case KEY_BACKSPACE:
			if (mode == EditorMode::Insert) {
				if(E.getNumRows()>0){
					TextRow& currRow=E.getTextRow(currCursorFileY);

					if(currRow.size>0 && currCursorFileX>0){
						currRow.text.erase(currCursorFileX-1,1);  // Delete one character before the cursor
						currRow.size = currRow.text.size();

						E.markAsDirty(); // manually mark as dirty since the text was modified outside of an EditorState method
						E.setCursorFileX(currCursorFileX - 1);
					}else if(currCursorFileX==0 && currCursorFileY>0){ // pressing backspace at the beginning of a line
						std::string currLineText=currRow.text;

						TextRow& prevRow=E.getTextRow(currCursorFileY-1);

						int newCursorFileX=prevRow.size;

						prevRow.text+=currLineText;
						prevRow.size=prevRow.text.size();

						E.removeRow(currCursorFileY);

						E.setCursorFileX(newCursorFileX);
						E.setCursorFileY(currCursorFileY-1);
					}

				}
			} else if(mode==EditorMode::Search){
			    const std::string& pattern=E.getCurrentPattern();
				int n=pattern.size();

				if(n>0){
				    E.updateSearchPattern(pattern.substr(0,n-1));
				}

			}
			break;
		case 9:{
		    if(mode==EditorMode::Insert){
    		    char ch='\t';
    			E.editorInsertChar(ch,E.getCursorFileY(),E.getCursorFileX());
    			E.setCursorFileX(E.getCursorFileX()+1);
			}else if(mode==EditorMode::Search){
                std::string currPattern=E.getCurrentPattern();
                for(std::size_t i=0;i<4;i++){
                    currPattern+=' ';
                }
                E.updateSearchPattern(currPattern);
			}
		}

		case ('s' & 0x1F): // This is CTRL+S
			updateFile();
			break;

		default:
			if(isprint(ch) && mode == EditorMode::Insert){ 	// Text insertion
				E.editorInsertChar(ch,E.getCursorFileY(),E.getCursorFileX());
				E.setCursorFileX(E.getCursorFileX()+1);

			}

			if(isprint(ch) && mode==EditorMode::Search){
			    std::string currPattern=E.getCurrentPattern();
				currPattern+=ch;
			    E.updateSearchPattern(currPattern);
			}
			break;
	}

	if(mode==EditorMode::Insert || mode == EditorMode::Normal){
	    E.fileCoordinatesToScreenCoordinates();
	}
}


int main(int argc, char* argv[])
{
	initscr();
	raw();
	noecho();
	keypad(stdscr, TRUE);
	clear();

	initEditor();

	printf("\x1b[2 q"); // Set initial cursor shape to block for Normal mode
    fflush(stdout);

	if(argc>=2){
		handleFile(argv);
	}else{
		die("Error: No filename provided.\n");
	}

	refresh();

	while(1){
		editorProcessKeypress();
		refreshScreen();
	}

    return 0;

}