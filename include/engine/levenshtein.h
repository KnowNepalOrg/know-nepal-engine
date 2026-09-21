#pragma once

#include <string_view>

namespace engine {

// Computes Levenshtein edit distance between two strings.
// Internally converts UTF-8 to UTF-16 code units before comparison,
// matching JavaScript/TypeScript string.length and a[i] semantics.
// This ensures behavioral parity with the TypeScript reference implementation.
int levenshtein(std::string_view a, std::string_view b);

// Computes similarity as 1.0 - (levenshtein / maxLen).
// Returns 1.0 if both strings are empty.
// Operates on UTF-16 code units internally.
double similarity(std::string_view a, std::string_view b);

}  // namespace engine
