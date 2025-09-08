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
		int getcursorX(){return cursorX;}
		int getcursorY(){return cursorY;}

		std::vector<textRow> getTextRows(){return row;}


		void setRows(int rows){terminalRows=rows;}
		void setCols(int cols){terminalCols=cols;}
		void setRowOffset(int val){rowOffset=val;}
		void setColOffset(int val){colOffset=val;}
		void setcursorX(int val){cursorX=val;}
		void setcursorY(int val){cursorY=val;}


		int getWindowSize(){
			int rows,cols;
			getmaxyx(stdscr,cols,rows);

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

	E.setcursorY(r);
	E.setcursorX(c);
}


void drawContent(){
	int r=E.getcursorY();
	int c=E.getcursorX();
	printw("%s",aBuf.getBuffer().c_str());
	move(r,c);
	aBuf.clear();
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
		aBuf.append(currentLine);
		aBuf.append("\n");
	}

	drawContent();
}


void refreshScreen(){
	clear();
	aBuf.clear();

	std::vector<textRow> textRows=E.getTextRows();


	for(std::size_t i=0;i<textRows.size();i++){
		if(i>=E.getRowOffset()){
			aBuf.append(textRows[i].text);
			aBuf.append("\n");
		}
	}

	drawContent();
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



