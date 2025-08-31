#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char* argv[])
{
    std::string filename{argv[1]};


    std::ifstream file{filename};

    if(!file){
        std::cerr<<"file couldnt be read!!\n";
        return 1;
    }

    std::string currentLine{};

    while(std::getline(file,currentLine))
        std::cout<<currentLine<<"\n";
    
    return 0;

}