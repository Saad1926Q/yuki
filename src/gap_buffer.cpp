#include "gap_buffer.hpp"
#include "types.hpp"
#include "utf8.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>

// Move forward by exactly one grapheme cluster.
//
// Sets limit to gapStart.byte when p is in segment A so nextGraphemeByte
// never reads gap bytes. gapStart is always on a cluster boundary ,
// so the function stops exactly at the end of a complete cluster - never
// mid-cluster. We then jump p.byte over the gap to land at segment B.
Pos GapBuffer::advance(Pos p){
    BytePos gapEnd = gapStart.byte + gapSize;
    BytePos limit  = (p.byte < gapStart.byte) ? gapStart.byte : buffer.size();
    BytePos next   = nextGraphemeByte(buffer.data(), p.byte, limit);

    if (next == gapStart.byte)
        next = gapEnd;

    return { .chr = p.chr + 1, .byte = next };
}

// Move backward by exactly one grapheme cluster.
//
// When p is at gapEnd (start of segment B), remap it to gapStart.byte so
// prevGraphemeByte walks through segment A, not across the gap. The lower
// bound is gapEnd when p is in segment B, preventing any backward read into
// the gap from that side.
Pos GapBuffer::retreat(Pos p){
    BytePos gapEnd = gapStart.byte + gapSize;
    BytePos lower = (p.byte > gapEnd) ? gapEnd : 0;
    BytePos raw = (p.byte == gapEnd) ? gapStart.byte : p.byte;
    BytePos prev = prevGraphemeByte(buffer.data(), raw, lower);

    return { .chr = p.chr-1, .byte = prev };
}

GapBuffer::GapBuffer(std::ifstream& file){
    // File must be opened in binary mode (std::ios::binary) by the caller.
    // We read raw bytes and do not interpret newlines or any encoding here.
    if (!file) {
        std::cerr << "file couldnt be read!!\n";
        std::exit(EXIT_FAILURE);
    }

    file.seekg(0, std::ios::end);
    std::streampos size = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer.resize(static_cast<size_t>(size) + GAP_BYTES_MIN);
    file.read(reinterpret_cast<char*>(buffer.data()), static_cast<size_t>(size));

    gapStart.byte = static_cast<size_t>(size);
    gapStart.chr  = 0;
    gapSize = GAP_BYTES_MIN;

    // Walk segment A to count grapheme clusters. advance() handles the gap automatically
    // After this loop, p.chr is the total cluster count.
    Pos p = { .chr = 0, .byte = 0 };

    while (p.byte < gapStart.byte)
        p = advance(p);

    gapStart.chr = p.chr;
    textEnd.byte = gapStart.byte;  // segment B is empty at construction
    textEnd.chr = p.chr;
}

// Return the byte offset of the nth grapheme cluster.
BytePos GapBuffer::charToByte(CharPos n){
    Pos p = { .chr = 0, .byte = 0 };

    while (p.chr < n)
        p = advance(p);

    return p.byte;
}

// Return the grapheme cluster index for a given byte offset.
CharPos GapBuffer::byteToChar(BytePos n){
    Pos p = { .chr = 0, .byte = 0 };

    while (p.byte < n)
        p = advance(p);

    return p.chr;
}

// Slide the gap so its left edge is at target. Direction is inferred from
// whether target is to the right or left of the current gapStart.
//
// gapSize never changes - the gap slides, it does not grow or shrink here.
void GapBuffer::shiftGap(Pos target){

    if (target.byte == gapStart.byte)
        return;

    BytePos gapEnd = gapStart.byte + gapSize;
    BytePos bytesToMove;

    if (target.byte > gapStart.byte) {
        // Shift right: copy the bytes that were just after the gap to just before it.
        bytesToMove = target.byte - gapStart.byte;
        memmove(buffer.data() + gapStart.byte, buffer.data() + gapEnd, bytesToMove);
    } else {
        // Shift left: copy the bytes that were just before the gap to just after it.
        bytesToMove = gapStart.byte - target.byte;
        memmove(buffer.data() + gapEnd - bytesToMove, buffer.data() + target.byte, bytesToMove);
    }

    gapStart = target;
}

// Grow the gap to fit at least nbytesNeeded bytes. Rebuilds the buffer in a
// new allocation with a larger gap at the same position, then swaps it in.
//
// Growth formula mirrors Emacs insdel.c:
//   new_gap = max(nbytesNeeded, textEnd.byte / 64) + GAP_BYTES_DFL

void GapBuffer::expand(std::size_t nbytesNeeded){
    std::size_t newGapSize = std::max(nbytesNeeded, textEnd.byte / 64) + GAP_BYTES_DFL;

    std::vector<uint8_t> newbuf;
    newbuf.reserve(buffer.size() + newGapSize);

    // Segment A - bytes before the gap
    newbuf.insert(newbuf.end(), buffer.begin(), buffer.begin() + gapStart.byte);
    // New gap - zeroed
    newbuf.resize(newbuf.size() + newGapSize, 0);
    // Segment B - bytes after the old gap
    newbuf.insert(newbuf.end(), buffer.begin() + gapStart.byte + gapSize, buffer.end());

    buffer = std::move(newbuf);
    gapSize = newGapSize;
}

// Insert one UTF-8 encoded codepoint (cpLen bytes) at cursor.
//
// Steps:
//   1. Validate cursor - throws if it's inside the gap or out of bounds.
//   2. Slide the gap to cursor so the insertion point is at gapStart.
//   3. Grow the gap if it can't fit cpLen bytes.
//   4. Copy the codepoint bytes into the gap and shrink the gap from the left.
//   5. Composing check: if the new codepoint attaches to the cluster just
//      before it (e.g. a combining accent after 'e'), do NOT increment the
//      chr counts - the byte count still grows
void GapBuffer::insert(Pos cursor, uint8_t* cpBytes, int cpLen){
    validatePos(cursor);

    shiftGap(cursor);

    if (static_cast<std::size_t>(cpLen) > gapSize)
        expand(static_cast<std::size_t>(cpLen));

    memcpy(buffer.data() + gapStart.byte, cpBytes, cpLen);

    // Shrink the gap from the left: advance gapStart past the new bytes.
    gapStart.byte += cpLen;
    gapSize -= cpLen;

    // Decode the codepoint we just inserted.
    uint32_t newCp = decodeCp(buffer.data(), gapStart.byte - cpLen, gapStart.byte);

    bool composing;

    if (gapStart.byte - cpLen > 0) {
        // Not the first codepoint in the buffer - check if newCp composes with the one before it.
        BytePos prevStart = prevCodepointByte(buffer.data(), gapStart.byte - cpLen);
        uint32_t prevCp = decodeCp(buffer.data(), prevStart, gapStart.byte);
        composing = isComposing(prevCp, newCp, nullptr);
    } else {
        // First codepoint in the buffer - nothing before it, check if it is inherently composing.
        composing = isComposingFirst(newCp);
    }

    if (!composing) {
        gapStart.chr += 1;
        textEnd.chr += 1;
    }

    textEnd.byte += cpLen;
}

// Delete one grapheme cluster at cursor.
// forward=true deletes the cluster after the cursor, forward=false deletes
// the cluster before it.
void GapBuffer::deleteChar(Pos cursor, bool forward){
    validatePos(cursor);

    shiftGap(cursor);

    if(forward){
        BytePos gapEnd = gapStart.byte + gapSize;

        // Find the start of the next cluster after gapEnd.
        BytePos next=nextGraphemeByte(buffer.data(),gapEnd,buffer.size());

        // Widen the gap rightward to swallow the cluster on the right side of the cursor. gapStart unchanged.
        BytePos delta = next-gapEnd;
        gapSize += delta;

        textEnd.byte -= delta;
        textEnd.chr -= 1;
    }else{
        // Find the start of the cluster ending at gapStart.
        BytePos prev = prevGraphemeByte(buffer.data(),gapStart.byte,0);

        // Widen the gap leftward to swallow the cluster on the left side of the cursor. gapStart moves back.
        BytePos delta = gapStart.byte-prev;
        gapStart.byte -= delta;
        gapStart.chr -= 1;
        gapSize += delta;

        textEnd.byte -= delta;
        textEnd.chr -= 1;
    }
}
