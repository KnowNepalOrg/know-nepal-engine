#include "engine/levenshtein.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace engine {

// Converts a UTF-8 string to a vector of UTF-16 code units.
// This matches JavaScript/TypeScript string representation where each
// character is a UTF-16 code unit (or surrogate pair for codepoints > U+FFFF).
// For Devanagari (U+0900-U+097F), each character maps to a single UTF-16 code unit.
static std::u16string utf8_to_utf16(std::string_view utf8) {
    std::u16string result;
    result.reserve(utf8.size());
    size_t i = 0;
    while (i < utf8.size()) {
        auto byte = static_cast<unsigned char>(utf8[i]);
        char32_t cp;
        int seq_len;

        if (byte < 0x80) {
            cp = byte;
            seq_len = 1;
        } else if ((byte & 0xE0) == 0xC0) {
            cp = byte & 0x1F;
            seq_len = 2;
        } else if ((byte & 0xF0) == 0xE0) {
            cp = byte & 0x0F;
            seq_len = 3;
        } else if ((byte & 0xF8) == 0xF0) {
            cp = byte & 0x07;
            seq_len = 4;
        } else {
            i += 1;
            continue;
        }

        for (int j = 1; j < seq_len; ++j) {
            if (i + j >= utf8.size()) {
                cp = 0xFFFD;
                break;
            }
            auto next = static_cast<unsigned char>(utf8[i + j]);
            if ((next & 0xC0) != 0x80) {
                cp = 0xFFFD;
                break;
            }
            cp = (cp << 6) | (next & 0x3F);
        }

        i += seq_len;

        // Encode as UTF-16
        if (cp <= 0xFFFF) {
            result.push_back(static_cast<char16_t>(cp));
        } else if (cp <= 0x10FFFF) {
            // Surrogate pair
            cp -= 0x10000;
            result.push_back(static_cast<char16_t>(0xD800 + (cp >> 10)));
            result.push_back(static_cast<char16_t>(0xDC00 + (cp & 0x3FF)));
        } else {
            result.push_back(0xFFFD);
        }
    }
    return result;
}

int levenshtein(std::string_view a, std::string_view b) {
    auto a16 = utf8_to_utf16(a);
    auto b16 = utf8_to_utf16(b);
    size_t len_a = a16.size();
    size_t len_b = b16.size();

    if (len_a == 0) return static_cast<int>(len_b);
    if (len_b == 0) return static_cast<int>(len_a);

    // Two-row DP to save memory
    std::vector<int> prev(len_b + 1);
    std::vector<int> curr(len_b + 1);

    for (size_t j = 0; j <= len_b; ++j) {
        prev[j] = static_cast<int>(j);
    }

    for (size_t i = 1; i <= len_a; ++i) {
        curr[0] = static_cast<int>(i);
        for (size_t j = 1; j <= len_b; ++j) {
            int cost = (a16[i - 1] == b16[j - 1]) ? 0 : 1;
            curr[j] = std::min({
                prev[j] + 1,       // deletion
                curr[j - 1] + 1,   // insertion
                prev[j - 1] + cost  // substitution
            });
        }
        std::swap(prev, curr);
    }

    return prev[len_b];
}

double similarity(std::string_view a, std::string_view b) {
    auto a16 = utf8_to_utf16(a);
    auto b16 = utf8_to_utf16(b);
    size_t max_len = std::max(a16.size(), b16.size());
    if (max_len == 0) return 1.0;
    return 1.0 - static_cast<double>(levenshtein(a, b)) / static_cast<double>(max_len);
}

}  // namespace engine
