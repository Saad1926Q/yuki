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
};


class EditorState{
	int terminalRows;
	int terminalCols;
	int numRows;
	std::vector<textRow> row;
	

	public:
		EditorState(){
			numRows=0;
		}
	
		int getRows(){return terminalRows;}
		int getCols(){return terminalCols;}

		void setRows(int rows){terminalRows=rows;}
		void setCols(int cols){terminalCols=cols;}


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

	getyx(stdscr,r,c);

	switch(ch){
		case KEY_UP:
			if(r!=0)
				r-=1;
			break;
		case KEY_DOWN:
			if(r!=LINES-1)
				r+=1;
			break;

		case KEY_RIGHT:
			if(c!=COLS-1)
				c+=1;
			break;
		
		case KEY_LEFT:
			if(c!=0)
				c-=1;
			break;

		case KEY_F(1):
			endwin();
			std::exit(0);
			break;
	}

	move(r,c);

}

void handleFile(char* argv[]){
	std::string filename{argv[1]};
	
	std::ifstream file{filename};

    if(!file){
        std::cerr<<"file couldnt be read!!\n";
        std::exit(EXIT_FAILURE);
    }

	std::string currentLine{};
	int curr_row=0;

	while(std::getline(file,currentLine)){
		mvprintw(curr_row,0,"%s",currentLine.c_str());
		E.appendRow(currentLine);

		curr_row++;

		if(curr_row>=LINES-1)
			break;
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

	if(argc>=2){
		handleFile(argv);
	}else{
		mvprintw(0,0,"Welcome to Sora Editor --version 1.0");
	}

	refresh();

	while(1){
		editorProcessKeypress();
	}
    
    return 0;

}



