#include "gap_buffer.hpp"
#include <fstream>
#include <iostream>


GapBuffer::GapBuffer(std::ifstream& file){
    /*

    Initializing a gap buffer from a file.


     */

    if(!file){
        std::cerr<<"file couldnt be read!!\n";
        std::exit(EXIT_FAILURE);
    }

    file.seekg(0, std::ios::end);

    std::streampos size = file.tellg();

    file.seekg(0, std::ios::beg);

    buffer.resize(static_cast<size_t>(size)+GAP_SIZE_BYTES);

    file.read(reinterpret_cast<char*>(buffer.data()),static_cast<size_t>(size));

    gapStart=static_cast<size_t>(size);
    gapEnd=static_cast<size_t>(size)+GAP_SIZE_BYTES;


}

void GapBuffer::shiftLeft(int cursor){
    int elementsToShift=gapStart-cursor;

    std::copy(buffer.begin()+cursor,buffer.begin()+cursor+elementsToShift,buffer.begin()+gapEnd-elementsToShift+1);

    gapStart=gapStart-elementsToShift;

    gapEnd=gapEnd-elementsToShift;

}
