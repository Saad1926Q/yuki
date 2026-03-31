#include "utf8.hpp"
#include "error.hpp"
#include "types.hpp"
#include "utf8proc.h"
#include <cstddef>
#include <cstdint>

// Given the byte position of a sequence start, how many bytes is the codepoint that starts there
// Uses the lenient table - continuation bytes (0x80-0xBF) and invalid lead bytes return 1, not 0. This prevents getting stuck during traversal of corrupt data.
int seqLen(uint8_t b){
    return lookupLenient[b];
}

// Returns true if b is a UTF-8 continuation byte (pattern 10xxxxxx).
bool isContinuationByte(uint8_t b){
    return (b & 0xC0)==0x80;
}

// Given a byte position pointing at the start of a codepoint,
// returns the byte position of the next codepoint.
BytePos nextCodepointByte(uint8_t* buf,BytePos pos){
    return pos+seqLen(buf[pos]);
}

// Given a byte position, returns the byte position where the previous codepoint starts.
// Walks backward past continuation bytes (10xxxxxx) until a leading byte is found.
// The caller is expected to call only for pos > 0, otherwise die() is called.
BytePos prevCodepointByte(uint8_t* buf,BytePos pos){

    if(pos == 0) die("prevCodepointByte: called with pos 0");

    --pos;

    while(pos>0 && isContinuationByte(buf[pos])){
        --pos;
    }

    return pos;
}

// Helper for decodeCp
constexpr uint32_t S(int s) {
      return 0x80U << s;
}


// Decodes the UTF-8 codepoint at buf[pos] and returns it as a uint32_t.
// Fast-path: ASCII bytes (< 0x80) are returned directly.
// For multibyte sequences, validates each continuation byte before decoding.
// On any validation failure, returns the raw first byte
//
// Does not advance pos - caller is responsible for calling nextCodepointByte afterwards if needed.
uint32_t decodeCp(uint8_t* buf,BytePos pos,std::size_t bufLen){
    uint32_t v0=buf[pos];

    if(v0<0x80U){  // it's a single-byte ASCII character. Return immediately.
        return v0;
    }

    const uint8_t len=lookupLenient[v0];

    if(len<2 || (pos + len > bufLen)){
        return v0;
    }

    uint32_t v1=buf[pos+1];

    if(!isContinuationByte((uint8_t)v1)) return v0;

    if(len==2) return (v0 << 6) + v1 - ((0xC0U << 6) + S(0));

    uint32_t v2=buf[pos+2];

    if(!isContinuationByte((uint8_t)v2)) return v0;

    if(len==3) return (v0 << 12) + (v1 << 6) + v2 - ((0xE0U << 12) + S(6) + S(0));

    uint32_t v3=buf[pos+3];

    if(!isContinuationByte((uint8_t)v3)) return v0;

    return (v0 << 18) + (v1 << 12) + (v2 << 6) + v3 - ((0xF0U << 18) + S(12) + S(6) + S(0));
}

// Returns true if the bytes at buf[pos] form a well-formed UTF-8 sequence.
// Uses the strict lookup table - returns false for continuation bytes and invalid bytes.
bool isValidSeq(uint8_t* buf, BytePos pos, std::size_t bufLen) {
    if (pos >= bufLen) return false;

    uint8_t val=buf[pos];

    int len=lookupStrict[val];

    if(len==0 || ((pos+len)>bufLen)) return false;

    for(std::size_t i=pos+1;i<pos+len;i++){
        val=buf[i];

        if(!isContinuationByte(val))return false;
    }

    return true;



}

// Returns true if codepoints one and two form an Arabic ligature.
bool arabic_combine(uint32_t one, uint32_t two){
    if(one==ArabicChar::LAM){
        return two==ArabicChar::ALEF_MADDA || two==ArabicChar::ALEF_HAMZA_ABOVE || two==ArabicChar::ALEF_HAMZA_BELOW||two==ArabicChar::ALEF;
    }

    return false;
}

// Check if the character pointed to by p2 is a composing character when it comes after p1.
bool isComposing(uint32_t prevCp, uint32_t cp, int32_t* state){
    if(cp<128)return false;

    if (!utf8proc_grapheme_break_stateful(prevCp, cp, state)) {
      return true;
    }

    return arabic_combine( prevCp,  cp);

}

bool isComposingFirst(uint32_t cp){
    return cp >= 128 && !utf8proc_grapheme_break(' ', cp);
}

// Given a byte position at the start of a grapheme cluster, returns the byte position
// of the next grapheme cluster.
// This function is gap-unaware - the caller passes limit to keep the scan
// within one contiguous segment (gapStart.byte for segment A, buffer.size() for segment B).
// limit is exclusive - the first byte that cannot be read. Valid reads are [pos, limit).
BytePos nextGraphemeByte(uint8_t* buf, BytePos pos, BytePos limit){
    std::uint8_t b0=buf[pos];

    // Fast-path: an ASCII byte can never be a composing character, but it CAN be a
    // base character followed by a non-ASCII combining mark (e.g. 'e' + U+0301).
    // We only skip the loop when the next byte is also ASCII, which guarantees
    // no combining character follows.
    if(b0<0x80 && (pos+1>=limit||buf[pos+1]<0x80))
        return pos+1;

    int len = seqLen(buf[pos]);

    // Invalid leading byte or sequence runs past limit - advance by 1 to keep moving.
    if((len == 1 && b0 >= 0x80)||(pos+len>limit))
        return pos+1;

    // prevCp tracks the last accepted codepoint
    uint32_t prevCp = decodeCp(buf, pos, limit);
    int nextBytePos=pos+len;

    int32_t graphemeState=0;

    while(nextBytePos<limit){
        // Combining characters are always > U+007F, so an ASCII byte ends the cluster.
        if(buf[nextBytePos]<0x80)
            break;

        int nextCharLen=seqLen(buf[nextBytePos]);

        // Skip if the full sequence would run past limit.
        if(nextBytePos + nextCharLen > limit)
            break;

        uint32_t currCp=decodeCp(buf,nextBytePos,limit);

        if(!isComposing(prevCp, currCp,&graphemeState))
            break;

        prevCp = currCp;
        nextBytePos+=nextCharLen;
    }

    return nextBytePos;
}

// Returns true if bc always starts a new cluster regardless of what came before it.
// CONTROL characters are always their own cluster - nothing ever fuses with them.
// False negatives are intentional: if unsure, return false and let the backward scan
// keep going. A false positive (saying break when there isn't one) would be a bug.
// Taken from always_break() in Neovim's mbyte.c.
bool alwaysBreak(int bc){
    return (bc == UTF8PROC_BOUNDCLASS_CONTROL);
}

// Returns true if bc2 always starts a new cluster after bc1, regardless of any prior context.
// False negatives are intentional - causes the backward scan to overshoot leftward,
// which the forward validation pass then corrects precisely.
//
// Three cases where a break is guaranteed without needing state:
//   1. bc2 is OTHER and bc1 is not PREPEND - regular characters never have anything
//      fuse onto them from the left side
//   2. bc1 is CR/LF/CONTROL - these always break after themselves
//   3. bc2 is an emoji and bc1 is OTHER or another emoji - definitely a new cluster.
//      If bc1 were ZWJ this would NOT fire (might be a ZWJ sequence), so the scan
//      keeps going further back, past the whole sequence.
//
// Taken from always_break_two() in Neovim's mbyte.c.
bool alwaysBreakTwo(int bc1, int bc2){
    return ((bc1 != UTF8PROC_BOUNDCLASS_PREPEND && bc2 == UTF8PROC_BOUNDCLASS_OTHER)
            || (bc1 >= UTF8PROC_BOUNDCLASS_CR && bc1 <= UTF8PROC_BOUNDCLASS_CONTROL)
            || (bc2 == UTF8PROC_BOUNDCLASS_EXTENDED_PICTOGRAPHIC
                && (bc1 == UTF8PROC_BOUNDCLASS_OTHER
                    || bc1 == UTF8PROC_BOUNDCLASS_EXTENDED_PICTOGRAPHIC)));
}

// Given a byte position at the start of a grapheme cluster, returns the byte position
// where the previous grapheme cluster starts.
// lower is inclusive - the first byte of valid memory in this segment.
//
// The grapheme break algorithm (utf8proc_grapheme_break_stateful) requires state that
// is built up by iterating left to right from the start of a cluster. Going backward,
// that state is unavailable - so we cannot determine exact cluster boundaries in a
// single backward pass. This is handled in two phases:
//
//   1. Backward pass: use cheap stateless boundclass checks (alwaysBreakTwo) to find
//      a position that is guaranteed to be at or before the real cluster start.
//      May overshoot leftward - false negatives are intentional.
//
//   2. Forward pass: from that safe position, scan forward with nextGraphemeByte which
//      runs the full stateful algorithm, correctly handling ZWJ emoji sequences, flag
//      pairs, and Indic conjuncts. Step cluster by cluster until we land exactly at pos.
//
// Inspired by utf_head_off() in Neovim's mbyte.c.
BytePos prevGraphemeByte(uint8_t* buf, BytePos pos, BytePos lower){
    if(pos==lower)
        return lower;

    BytePos curr=prevCodepointByte(buf,pos);

    uint32_t currCp=decodeCp(buf,curr,pos);

    int currBc=utf8proc_get_property(currCp)->boundclass;

    // ASCII and CONTROL characters are always their own cluster.
    if(buf[curr]<0x80||alwaysBreak(currBc))
        return curr;

    if(curr==lower)
        return lower;

    // Backward pass: walk left using cheap boundclass checks (alwaysBreakTwo) to find
    // a position guaranteed to be at or before the real cluster start. May overshoot
    // leftward - the forward pass below corrects this. False negatives in alwaysBreakTwo
    // are intentional: when uncertain, keep going back rather than stopping too early.
    // Inspired by the backward heuristic scan in utf_head_off() in Neovim's mbyte.c.
    while(true){
        BytePos prev = prevCodepointByte(buf, curr);

        if(prev<lower)
            break;

        // ASCII to the left means curr is definitely a cluster start.
        if(buf[prev]<0x80)
            break;

        uint32_t prevCp = decodeCp(buf,prev,curr);

        int prevBc=utf8proc_get_property(prevCp)->boundclass;

        // Definite break between prev and curr - curr is the cluster start.
        if(alwaysBreakTwo(prevBc, currBc) && !arabic_combine(prevCp, currCp))
            break;

        curr=prev;
        currBc=prevBc;
        currCp=prevCp;

        if(curr==lower)
            break;
    }

    // Forward pass: scan forward from the backward pass result using nextGraphemeByte,
    // which has full stateful grapheme break support (handles ZWJ emoji sequences,
    // flag pairs, Indic conjuncts correctly). Step cluster by cluster until the next
    // boundary lands exactly at pos - that means q is the real cluster start.
    // Inspired by the forward validation pass in utf_head_off() in Neovim's mbyte.c.
    BytePos q=curr;

    while(q<pos){
        BytePos next=nextGraphemeByte(buf,q,pos);

        if(next==pos)
            return q;

        q=next;
    }

    die("prevGraphemeByte: forward scan did not land on pos - pos is not a cluster boundary");
}
