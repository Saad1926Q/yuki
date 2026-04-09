#pragma once

#include <cassert>
#include <cstdint>
#include <fstream>
#include <vector>
#include "utf8.hpp"
#include "types.hpp"

constexpr std::size_t GAP_BYTES_MIN = 20;   // initial gap size on construction
constexpr std::size_t GAP_BYTES_DFL = 2000; // Default padding added on every grow

// Bundles a byte offset and grapheme cluster index - always kept in sync.
struct Pos{
    CharPos chr;  // grapheme cluster index (what the user sees)
    BytePos byte; // raw byte offset into buffer memory
};

class GapBuffer{

    void validatePos(Pos p){
        BytePos gapEnd = gapStart.byte + gapSize;

        if (p.byte > gapStart.byte && p.byte < gapEnd)
            throw std::out_of_range("pos inside gap");

        // must not exceed text end
        if (p.byte > textEnd.byte || p.chr > textEnd.chr)
            throw std::out_of_range("pos beyond textEnd");

        // byte must be on a cluster boundary (not a continuation byte)
        if (p.byte > 0 && p.byte != gapEnd)
            assert(!isContinuationByte(buffer[p.byte]));
    }

    public:

        Pos gapStart; // first byte of the gap (outside real text, inside gap region)
        Pos textEnd;  // one past the end of all real text (excluding gap bytes), like std::end()

        std::size_t gapSize;   // gap length in bytes

        std::vector<uint8_t> buffer; // raw byte storage - includes gap bytes

        Pos advance(Pos p); // Move forward one grapheme cluster, jumping the gap if needed.

        GapBuffer(std::ifstream& file); // Initialize gap buffer with contents of a file

        Pos retreat(Pos p); // Move backward one grapheme cluster, jumping the gap if needed.

        BytePos charToByte(CharPos n); // Return byte offset of the nth grapheme cluster.

        CharPos byteToChar(BytePos n); // Return grapheme cluster index at the given byte offset.

        void shiftGap(Pos target); // Move the gap so its left edge sits at target.

        void expand(std::size_t nbytesNeeded); // Grow the gap when it is too small for an insertion.

        void insert(Pos cursor, uint8_t* cpBytes, int cpLen); // Insert one UTF-8 encoded codepoint at cursor.

        void deleteChar(Pos cursor, bool forward); // Delete one grapheme cluster. Widens the gap.


};
