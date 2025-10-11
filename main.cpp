#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ncurses.h>


struct textRow{
	int size;
	std::string text;
};

class AppendBuffer{
	std::string buffer;

	public:
		std::string getBuffer(){
			return buffer;
		}

		void append(const std::string& str){
			buffer+=str;
		}

		void clear(){
			buffer.clear();
		}
};


class EditorState{
	int terminalRows;
	int terminalCols;
	int numRows;
	int rowOffset;
	int colOffset;
	int cursorX;
	int cursorY;
	int cursorFileY;  // This tells us which row in the file the cursor is on
	int cursorFileX;  // This tells us which col in the file the cursor is on
	std::string filename;

	std::vector<textRow> textRows;

	public:
		EditorState(){
			numRows=0;
			rowOffset=0;
			colOffset=0;
			cursorX=0;
			cursorY=0;
			cursorFileY=0;
		}
	
		int getRows(){return terminalRows;}
		int getCols(){return terminalCols;}
		int getNumRows(){return numRows;}
		int getRowOffset(){return rowOffset;}
		int getColOffset(){return colOffset;}
		int getCursorX(){return cursorX;}
		int getCursorY(){return cursorY;}

		int getCursorFileY(){return cursorFileY;}
		int getCursorFileX(){return cursorFileX;}

		std::vector<textRow> getTextRows(){return textRows;}
		textRow &getTextRow(int idx){return textRows[idx];}

		std::string getFileName(){return filename;}


		void setRows(int rows){terminalRows=rows;}
		void setCols(int cols){terminalCols=cols;}
		void setRowOffset(int val){rowOffset=val;}
		void setColOffset(int val){colOffset=val;}
		void setCursorX(int val){cursorX=val;}
		void setCursorY(int val){cursorY=val;}
		void setCursorFileY(int val){cursorFileY=val;}
		void setCursorFileX(int val){cursorFileX=val;}
		void setFileName(std::string str){filename=str;}


		int getWindowSize(){

			int rows,cols;
			getmaxyx(stdscr,rows,cols);

			if(rows==0||cols==0)
				return -1;

			setRows(rows);
			setCols(cols);

			return 0;
		}

		void appendRow(std::string text){
			textRow txtRow;
			txtRow.text=text;
			txtRow.size=text.size();

			numRows+=1;
			textRows.push_back(txtRow);
		}

		void insertRow(std::string text,int pos){
			textRow newRow;
				
			newRow.text=text;
			newRow.size=text.size();

			textRows.insert(textRows.begin()+pos,newRow);  // Add new row
			numRows++;

		}

		void removeRow(int pos){
			textRows.erase(textRows.begin()+pos);
			numRows--;
		}

		void editorInsertChar(char ch,int r,int c){
			int numFileRows=getNumRows();

			if(r>=0 && r<numFileRows){
				textRow &row=textRows[r];

				if(c>=0 && c<row.size){
					row.text.insert(c,1,ch);
				}else if(c==row.size){
					row.text+=ch;
				}

				row.size++;

			}

		}
		


};

AppendBuffer aBuf;
EditorState E;


void die(std::string errorMessage){
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

	if(colOffset<rowContent.size()){
		std::string rowSubstr=rowContent.substr(colOffset);
		if(rowSubstr.size()>COLS)
			rowSubstr=rowSubstr.substr(0,COLS-1);

		for(char ch:rowSubstr){
			if(ch=='\t'){
				for(std::size_t i=0;i<TABSIZE;i++)
					bufferContent+=" ";
			}else{
				bufferContent+=ch;
			}
		}
	}

	bufferContent+="\n";
	aBuf.append(bufferContent);	
}


void handleFile(char* argv[]){
	std::string filename{argv[1]};
	
	std::ifstream file{filename};

	E.setFileName(filename);

    if(!file){
        std::cerr<<"file couldnt be read!!\n";
        std::exit(EXIT_FAILURE);
    }

	std::string currentLine{};

	int i=0;

	while(std::getline(file,currentLine)){
		E.appendRow(currentLine);
		appendLineToBuffer(currentLine);
	}


	if (E.getNumRows() == 0) 
	    E.appendRow("");
	

	drawContent();
}

void updateFile(){
	std::string filename=E.getFileName();

	std::ofstream file(filename);

	std::vector<textRow> textRows=E.getTextRows();

	for(std::size_t i=0;i<textRows.size();i++){
		file<<textRows[i].text;

		if(i!=textRows.size()-1)
			file<<"\n";
	}

	file.close();
}

void refreshScreen(){
	E.getWindowSize();

	curs_set(0);

	erase();
	aBuf.clear();

	int colOffset=E.getColOffset();
	std::vector<textRow> textRows=E.getTextRows();


	for(std::size_t i=0;i<textRows.size();i++){
		if(i>=E.getRowOffset()){
			std::string rowContent=textRows[i].text;
			appendLineToBuffer(rowContent,colOffset);
		}
	}

	drawContent();

	refresh();

	curs_set(1);
}


void editorProcessKeypress() {
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

					E.setCursorFileX(currCursorFileX-1);
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



