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

	


	std::vector<textRow> row;
	

	public:
		EditorState(){
			numRows=0;
			rowOffset=0;
			colOffset=0;
			cursorX=0;
			cursorY=0;
		}
	
		int getRows(){return terminalRows;}
		int getCols(){return terminalCols;}
		int getNumRows(){return numRows;}
		int getRowOffset(){return rowOffset;}
		int getColOffset(){return colOffset;}
		int getCursorX(){return cursorX;}
		int getCursorY(){return cursorY;}

		std::vector<textRow> getTextRows(){return row;}


		void setRows(int rows){terminalRows=rows;}
		void setCols(int cols){terminalCols=cols;}
		void setRowOffset(int val){rowOffset=val;}
		void setColOffset(int val){colOffset=val;}
		void setCursorX(int val){cursorX=val;}
		void setCursorY(int val){cursorY=val;}


		int getWindowSize(){

			int rows,cols;
			getmaxyx(stdscr,rows,cols);

			if(rows==0||cols==0)
				return -1;

			setRows(cols);
			setCols(rows);

			return 0;
		}

		void appendRow(std::string text){
			textRow txtRow;
			txtRow.text=text;
			txtRow.size=text.size();

			numRows+=1;
			row.push_back(txtRow);
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


void editorProcessKeypress() {
	int ch=getch();

	int r,c;
	int rowOffset=E.getRowOffset();
	int colOffset=E.getColOffset();

	int terminalRows=E.getRows();
	int terminalCols=E.getCols();

	getyx(stdscr,r,c);

	switch(ch){
		case KEY_UP:
			if(r!=0)
				r-=1;
			else if(r==0 && rowOffset!=0)
				E.setRowOffset(rowOffset-1);
			break;


		case KEY_DOWN:
			if(r!=LINES-1)
				r+=1;
			else if(r==LINES-1 && rowOffset!=E.getNumRows()-1)
				E.setRowOffset(rowOffset+1);
			break;

		case KEY_RIGHT:
			if(c!=COLS-1)
				c+=1;
			else
				E.setColOffset(colOffset+1);
			break;
		
		case KEY_LEFT:
			if(c!=0)
				c-=1;
			else if(c==0 and colOffset!=0)
				E.setColOffset(colOffset-1);
			break;

		case KEY_F(1):
			endwin();
			std::exit(0);
			break;
	}

	E.setCursorY(r);
	E.setCursorX(c);
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

	drawContent();
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
		mvprintw(0,0,"Welcome to Sora Editor --version 1.0");
	}

	refresh();

	while(1){
		editorProcessKeypress();
		refreshScreen();
	}
    
    return 0;

}



