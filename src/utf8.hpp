#ifndef UTF8_HPP
#define UTF8_HPP

// yuki works with UTF-8 encoded text and thinks about content at two levels:
//
//   Bytes - the raw memory representation. A single Unicode codepoint can be
//   1–4 bytes wide in UTF-8, and what the user perceives as one "character"
//   may span several codepoints.
//
//   Grapheme clusters - what the user actually sees and interacts with: one
//   visible "character" on screen. Cursor movement, deletion,
//   and selection all operate on grapheme clusters, not raw bytes.
//
// We therefore track two distinct position types throughout the codebase:
// BytePos for offsets into raw buffer memory, and CharPos for grapheme-cluster
// positions as the user perceives them.


#include "types.hpp"
#include <array>
#include <cstdint>
#include <utf8proc.h>

// Everytime we want to know a character's length, we will simply refer to our lookup tables.

// Lookup tables - the first byte alone tells you how long the whole sequence is.
//   0xxxxxxx  ->  ASCII
//   110xxxxx  ->  lead byte of 2-byte char
//   1110xxxx  ->  lead byte of 3-byte char
//   11110xxx  ->  lead byte of 4-byte char
//   10xxxxxx  ->  continuation byte

// This will allow us to know the character's length in constant time.
// The idea behind these lookup tables was taken from neovim codebase.

// Lenient lookup table - continuation bytes and invalid lead bytes return 1, never 0.
// Use this during buffer traversal where corrupt data should not get us stuck.

constexpr std::array<uint8_t, 256> lookupLenient = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1,
};


// Strict lookup table - returns 0 for continuation bytes and invalid lead bytes.
// Use this when validating input before inserting.

constexpr std::array<uint8_t, 256> lookupStrict = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 0, 0,
};

enum ArabicChar : uint32_t {
      ALEF_MADDA       = 0x0622,
      ALEF_HAMZA_ABOVE = 0x0623,
      ALEF_HAMZA_BELOW = 0x0625,
      ALEF             = 0x0627,
      LAM              = 0x0644,
};

int seqLen(uint8_t b);

bool isContinuationByte(uint8_t b);

BytePos nextCodepointByte(uint8_t* buf, BytePos pos);

BytePos prevCodepointByte(uint8_t* buf, BytePos pos);

uint32_t decodeCp(uint8_t* buf,BytePos pos,std::size_t bufLen);

bool isValidSeq(uint8_t* buf, BytePos pos, std::size_t bufLen) ;

bool arabic_combine(uint32_t one, uint32_t two);

bool isComposing(uint32_t prevCp, uint32_t cp, int32_t* state);

bool isComposingFirst(uint32_t cp);

BytePos nextGraphemeByte(uint8_t* buf, BytePos pos, BytePos limit);

BytePos prevGraphemeByte(uint8_t* buf, BytePos pos, BytePos lower);

#endif
