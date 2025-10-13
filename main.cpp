#include <iostream>
#include <fstream>
#include <ncurses.h>
#include "editor.hpp"

AppendBuffer aBuf;
EditorState E;


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
                for (int i = 0; i < TABSIZE; ++i)
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

	std::vector<textRow> textRows=E.getTextRows();

	for(std::size_t i=0;i<textRows.size();i++){
		file<<textRows[i].text;

		if(i!=textRows.size()-1)
			file<<"\n";
	}

	file.close();
	E.onFileSave(); // to reset the dirty flag after saving the file
}

void refreshScreen(){ //Every cycle update the Append buffer and redraws the content on the screen 
    E.getWindowSize();
    curs_set(0);

    erase();
    aBuf.clear();

    int colOffset=E.getColOffset();
    std::vector<textRow> textRows=E.getTextRows();

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

    refresh();

    curs_set(1);
}


void editorProcessKeypress() {  // Main function which handles the different key press events
	int ch=getch();

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

	switch(ch){
		case KEY_UP:
			if(currCursorFileY!=0){				
				E.setCursorFileY(currCursorFileY-1); // Move one line up

				textRow& newRow = E.getTextRow(currCursorFileY-1);
				
				if(currCursorFileX>newRow.size)
					E.setCursorFileX(newRow.size); // If the length of new line is smaller than the previous line then snap to the end of the new line
				
			}
			break;

		case KEY_DOWN:
			if(currCursorFileY<E.getNumRows()-1){
				E.setCursorFileY(currCursorFileY+1); // Move one line down
				
				textRow& newRow = E.getTextRow(E.getCursorFileY());
				
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
			if(E.getNumRows()>0){
				textRow& currRow=E.getTextRow(currCursorFileY);
				std::string newLineText=currRow.text.substr(currCursorFileX);  // This is the text that will become the next line

				currRow.text=currRow.text.substr(0,currCursorFileX);  // Shorten the original line
				currRow.size = currRow.text.size(); // Update the size of the curr row

				E.insertRow(newLineText,currCursorFileY+1);

				E.setCursorFileX(0); // Move cursor to the beginning of the new line
				E.setCursorFileY(currCursorFileY+1);

			}

			break;

		case KEY_BACKSPACE:
			if(E.getNumRows()>0){
				textRow& currRow=E.getTextRow(currCursorFileY);

				if(currRow.size>0 && currCursorFileX>0){
					currRow.text.erase(currCursorFileX-1,1);  // Delete one character before the cursor
					currRow.size = currRow.text.size();

					E.markAsDirty(); // manually mark as dirty since the text was modified outside of an EditorState method
					E.setCursorFileX(currCursorFileX - 1);
				}else if(currCursorFileX==0 && currCursorFileY>0){ // pressing backspace at the beginning of a line
					std::string currLineText=currRow.text;
					
					textRow& prevRow=E.getTextRow(currCursorFileY-1);
					
					int newCursorFileX=prevRow.size;

					prevRow.text+=currLineText;
					prevRow.size=prevRow.text.size();

					E.removeRow(currCursorFileY);

					E.setCursorFileX(newCursorFileX);
					E.setCursorFileY(currCursorFileY-1);
				}

			}
			break;

		case ('s' & 0x1F): // This is CTRL+S
			updateFile();
			break;

		default:
			if(isprint(ch)){ 	// Text insertion 
				E.editorInsertChar(ch,E.getCursorFileY(),E.getCursorFileX());
				E.setCursorFileX(E.getCursorFileX()+1);

			}
			break;
	}

	currCursorFileY = E.getCursorFileY();
    currCursorFileX = E.getCursorFileX();

	if(currCursorFileY<E.getRowOffset()){
		E.setRowOffset(currCursorFileY);
	}
	
	if(currCursorFileY>=E.getRowOffset()+terminalRows){
		E.setRowOffset(currCursorFileY-terminalRows+1);
	}

	if(currCursorFileX < E.getColOffset()){
		E.setColOffset(currCursorFileX);
	}
	
	if(currCursorFileX>=E.getColOffset()+terminalCols){
		E.setColOffset(currCursorFileX-terminalCols+1);
	}



	rowOffset = E.getRowOffset();  // Get the offset which might have been updated
	colOffset = E.getColOffset(); // Get the offset which might have been updated

	int r = E.getCursorFileY() - rowOffset;
	int c = E.getCursorFileX() - colOffset;

	E.setCursorY(r);
	E.setCursorX(c);
}


int main(int argc, char* argv[])
{	
	initscr();
	raw();
	noecho();
	keypad(stdscr, TRUE);
	clear(); 

	initEditor();

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



