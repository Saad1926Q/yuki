#ifndef GAP_BUFFER_HPP
#define GAP_BUFFER_HPP

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <vector>

using BytePos =std::size_t;
using CharPos=std::size_t;

const int GAP_SIZE_BYTES=40;

class GapBuffer{
    public:

        BytePos gapStart; // gap start in bytes
        BytePos gapEnd; // gap end in bytes
        CharPos gapStartChar;  // gap start in grapheme clusters
        CharPos gapEndChar; // gap end in grapheme clusters

        std::size_t textBytes;          // total bytes of actual text (excl. gap)
        std::size_t textChars;          //total grapheme clusters of actual text (excl. gap)

        std::vector<uint8_t> buffer;


        GapBuffer(std::ifstream& file);  // Initialize gap buffer with contents of a file

        void shiftRight(int cursor); // Shift the gap buffer to the right

        void shiftLeft(int cursor); // Shift the gap buffer to the left

        void expand(); // Once entire gap buffer has been filled then expand it

};

#endif
