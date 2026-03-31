#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstddef>

// Position in raw buffer memory, counted in bytes.
using BytePos = std::size_t;

// Position as the user perceives it, counted in grapheme clusters.
using CharPos = std::size_t;

#endif
