#include "utf8.hpp"
#include <gtest/gtest.h>

static uint8_t ASCII[] = {'h','e','l','l','o'};
static uint8_t LATIN_COMB[] = {0x65, 0xCC, 0x81, 0x41};     // e + ◌́ + A  →  "éA"
static uint8_t TWO_BYTE[] = {0xC3, 0xA9};                  // U+00E9 → "é"
static uint8_t THREE_BYTE[] = {0xE2, 0x82, 0xAC};            // U+20AC → "€" (Euro sign)
static uint8_t FOUR_BYTE[] = {0xF0, 0x9F, 0x98, 0x80};      // U+1F600 → (grinning face emoji)

// ZWJ family
static uint8_t ZWJ_FAMILY[] = {
    0xF0,0x9F,0x91,0xA8, 0xE2,0x80,0x8D,
    0xF0,0x9F,0x91,0xA9, 0xE2,0x80,0x8D,
    0xF0,0x9F,0x91,0xA7
};

// two Regional Indicator codepoints, 4 bytes each = 8 bytes
static uint8_t FLAG_US[] = {0xF0,0x9F,0x87,0xBA, 0xF0,0x9F,0x87,0xB8};

// LAM (ل) + ALEF (ا) - Arabic ligature pair
static uint8_t ARABIC[]  = {0xD9,0x84, 0xD8,0xA7}; // لا


// Layer 0 Tests

// SeqLen tests

TEST(SeqLen, ASCII){ EXPECT_EQ(seqLen(0x41), 1);}
TEST(SeqLen, TwoByteLeadByte){ EXPECT_EQ(seqLen(0xC3), 2);}
TEST(SeqLen, ThreeByteLeadByte){ EXPECT_EQ(seqLen(0xE2), 3);}
TEST(SeqLen, FourByteLeadByte){ EXPECT_EQ(seqLen(0xF0), 4);}
TEST(SeqLen, ContinuationLenient){ EXPECT_EQ(seqLen(0x80), 1);}
TEST(SeqLen, InvalidLenient){ EXPECT_EQ(seqLen(0xFF), 1);}

// Continuation Byte Tests

TEST(IsContinuationByte, FirstContinuation){ EXPECT_TRUE(isContinuationByte(0x80));}
TEST(IsContinuationByte, LastContinuation){ EXPECT_TRUE(isContinuationByte(0xBF));}
TEST(IsContinuationByte, PlainASCII){ EXPECT_FALSE(isContinuationByte(0x41));}
TEST(IsContinuationByte, LeadByte){ EXPECT_FALSE(isContinuationByte(0xC0));}

// Layer 1 Tests

// DecodeCp Tests

TEST(DecodeCp, ASCII) {EXPECT_EQ(decodeCp(ASCII, 0, 5), 0x68u);}  // 'h'
TEST(DecodeCp, TwoByte) {EXPECT_EQ(decodeCp(TWO_BYTE, 0, 2), 0xE9u);}   // é
TEST(DecodeCp, ThreeByte) {EXPECT_EQ(decodeCp(THREE_BYTE, 0, 3), 0x20ACu);} // €
TEST(DecodeCp, FourByte) {EXPECT_EQ(decodeCp(FOUR_BYTE, 0, 4), 0x1F600u);}
TEST(DecodeCp, ReturnsRawLeadByte) {EXPECT_EQ(decodeCp(TWO_BYTE, 0, 1), 0xC3u);} // bufLen=1 means continuation bytes are inaccessible - returns raw lead byte

// nextCodepointByte Tests

TEST(NextCodepointByte, ASCII) {
    EXPECT_EQ(nextCodepointByte(ASCII,0), 1u);
    EXPECT_EQ(nextCodepointByte(ASCII,1), 2u);
}

TEST(NextCodepointByte, TwoByte){ EXPECT_EQ(nextCodepointByte(TWO_BYTE,0), 2u);}
TEST(NextCodepointByte, ThreeByte){ EXPECT_EQ(nextCodepointByte(THREE_BYTE,0), 3u);}
TEST(NextCodepointByte, FourByte){ EXPECT_EQ(nextCodepointByte(FOUR_BYTE,0), 4u);}

// prevCodepointByte Tests

TEST(PrevCodepointByte, ASCII) {
    EXPECT_EQ(prevCodepointByte(ASCII, 1), 0u);
    EXPECT_EQ(prevCodepointByte(ASCII, 2), 1u);
}

TEST(PrevCodepointByte, TwoByte){ EXPECT_EQ(prevCodepointByte(TWO_BYTE,2), 0u);}
TEST(PrevCodepointByte, ThreeByte){ EXPECT_EQ(prevCodepointByte(THREE_BYTE,3), 0u);}
TEST(PrevCodepointByte, FourByte){ EXPECT_EQ(prevCodepointByte(FOUR_BYTE,4), 0u);}

// ValidSeq Tests

TEST(IsValidSeq, ASCII){ EXPECT_TRUE(isValidSeq(ASCII,0, 5));}
TEST(IsValidSeq, TwoByte){ EXPECT_TRUE(isValidSeq(TWO_BYTE,0, 2));}
TEST(IsValidSeq, ThreeByte){ EXPECT_TRUE(isValidSeq(THREE_BYTE,0, 3));}
TEST(IsValidSeq, FourByte){ EXPECT_TRUE(isValidSeq(FOUR_BYTE,0, 4));}

TEST(IsValidSeq, TruncatedTwoByte) {EXPECT_FALSE(isValidSeq(TWO_BYTE, 0, 1));} // needs 2 bytes

TEST(IsValidSeq, ContinuationAtStart) {
    uint8_t buf[] = {0x80};
    EXPECT_FALSE(isValidSeq(buf, 0, 1)); // continuation byte can't start a sequence
}

TEST(IsValidSeq, BadContinuation) {
    uint8_t buf[] = {0xC3, 0x41};
    EXPECT_FALSE(isValidSeq(buf, 0, 2)); // valid lead, but 0x41 is not a continuation byte
}

// IsComposing Tests

TEST(IsComposing, CombiningAccentAfterE){ EXPECT_TRUE(isComposing(0x65, 0x0301, nullptr)); }
TEST(IsComposing, TwoPlainASCII){ EXPECT_FALSE(isComposing('A', 'B', nullptr)); }
TEST(IsComposing, ASCIICpFastPath){ EXPECT_FALSE(isComposing(0x0301, 0x41, nullptr)); } // 0x41 is ASCII ('A') - ASCII can never be a combining character regardless of what came before it
TEST(IsComposing, ArabicLamAlef){ EXPECT_TRUE(isComposing(ArabicChar::LAM, ArabicChar::ALEF, nullptr)); }
TEST(IsComposing, ArabicLamNonAlef){ EXPECT_FALSE(isComposing(ArabicChar::LAM, 0x0628, nullptr)); }

TEST(IsComposing, ZWJSequenceStateful){
    // 0x1F468 + 0x200D + 0x1F469 + 0x200D + 0x1F467 form a ZWJ sequence (family emoji)
    int32_t state = 0;
    EXPECT_TRUE(isComposing(0x1F468, 0x200D, &state));
    EXPECT_TRUE(isComposing(0x200D, 0x1F469, &state));
}


// IsComposingFirst Tests

TEST(IsComposingFirst, CombiningAccent){ EXPECT_TRUE(isComposingFirst(0x0301)); }  // combining acute accent - composing with no prior character
TEST(IsComposingFirst, ASCII){ EXPECT_FALSE(isComposingFirst(0x41)); } // 'A' - ASCII can never be composing
TEST(IsComposingFirst, NonASCIIRegularLetter){ EXPECT_FALSE(isComposingFirst(0x00C9)); } // 'É' - non-ASCII but a base letter, not combining

// NextGraphemeByte Tests

TEST(NextGraphemeByte, ASCIISimple){ EXPECT_EQ(nextGraphemeByte(ASCII, 0, 5), 1u); }
TEST(NextGraphemeByte, TwoByteCp){ EXPECT_EQ(nextGraphemeByte(TWO_BYTE, 0, 2), 2u); }
TEST(NextGraphemeByte, FourByteEmoji){ EXPECT_EQ(nextGraphemeByte(FOUR_BYTE, 0, 4), 4u); }
TEST(NextGraphemeByte, ComposingCluster){ EXPECT_EQ(nextGraphemeByte(LATIN_COMB, 0, 3), 3u); }  // e + U+0301 combining acute must be one cluster
TEST(NextGraphemeByte, LimitCutsBeforeCombiningStart){ EXPECT_EQ(nextGraphemeByte(LATIN_COMB, 0, 1), 1u); } // combining mark starts at gap boundary, e is alone
TEST(NextGraphemeByte, LimitCutsInsideCombiningSeq){ EXPECT_EQ(nextGraphemeByte(LATIN_COMB, 0, 2), 1u); } // lead byte 0xCC readable but continuation 0x81 is past gap, e is alone
TEST(NextGraphemeByte, ZWJFamilyEmoji){ EXPECT_EQ(nextGraphemeByte(ZWJ_FAMILY, 0, 18), 18u); } // stateful ZWJ chaining must consume all 18 bytes as one cluster
TEST(NextGraphemeByte, FlagPair){ EXPECT_EQ(nextGraphemeByte(FLAG_US, 0, 8), 8u); } // two Regional Indicators must fuse into one cluster

TEST(NextGraphemeByte, LeadingCombiningMark){
    uint8_t buf[] = {0xCC, 0x81};
    EXPECT_EQ(nextGraphemeByte(buf, 0, 2), 2u);
} // U+0301 with no base - standalone combining mark is still its own cluster

TEST(NextGraphemeByte, ArabicLigature){ EXPECT_EQ(nextGraphemeByte(ARABIC, 0, 4), 4u); } // LAM + ALEF must be consumed as one cluster

// PrevGraphemeByte Tests

TEST(PrevGraphemeByte, ASCIIStepBack){
    EXPECT_EQ(prevGraphemeByte(ASCII, 5, 0), 4u);
    EXPECT_EQ(prevGraphemeByte(ASCII, 4, 0), 3u);
    EXPECT_EQ(prevGraphemeByte(ASCII, 1, 0), 0u);
}

TEST(PrevGraphemeByte, BackOverComposingCluster){ EXPECT_EQ(prevGraphemeByte(LATIN_COMB, 3, 0), 0u); }

TEST(PrevGraphemeByte, BackOverZWJSequence){
    // ZWJ family (18 bytes) + 'A'; stepping back from pos=18 must clear the full ZWJ cluster
    uint8_t buf[] = {0xF0,0x9F,0x91,0xA8, 0xE2,0x80,0x8D, 0xF0,0x9F,0x91,0xA9, 0xE2,0x80,0x8D, 0xF0,0x9F,0x91,0xA7, 0x41};
    EXPECT_EQ(prevGraphemeByte(buf, 19, 0), 18u); // back over 'A'
    EXPECT_EQ(prevGraphemeByte(buf, 18, 0), 0u); // back over full 18-byte ZWJ cluster
}
