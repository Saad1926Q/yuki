#include <iostream>
#include <fstream>
#include <string>
#include <ncurses.h>


int main(int argc, char* argv[])
{
    std::string filename{argv[1]};


    std::ifstream file{filename};

    if(!file){
        std::cerr<<"file couldnt be read!!\n";
        return 1;
    }
	


	initscr();
	keypad(stdscr, TRUE);
	clear(); 


	std::string currentLine{};
	int curr_row=0;

	while(std::getline(file,currentLine)){
		mvprintw(curr_row,0,"%s",currentLine.c_str());

		curr_row++;

		if(curr_row>=LINES-1)
			break;
	}

	refresh();

	int ch;

	int r,c;

	
	while((ch = getch()) != KEY_F(1)){
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
		}

		move(r,c);
	}

    endwin();
    
    return 0;

}



