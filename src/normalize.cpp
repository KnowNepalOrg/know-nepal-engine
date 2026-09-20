#include "engine/normalize.h"

#include <algorithm>
#include <cctype>

namespace engine {

static bool is_devanagari(char32_t cp) {
    return cp >= 0x0900 && cp <= 0x097F;
}

static bool is_ascii_alnum(char32_t cp) {
    return (cp >= 'a' && cp <= 'z') || (cp >= '0' && cp <= '9');
}

static char32_t to_lower_ascii(char32_t cp) {
    if (cp >= 'A' && cp <= 'Z') {
        return cp + 32;
    }
    return cp;
}

static bool is_space(char32_t cp) {
    return cp == ' ' || cp == '\t' || cp == '\n' || cp == '\r';
}

static std::string codepoint_to_utf8(char32_t cp) {
    std::string result;
    if (cp < 0x80) {
        result.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        result.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        result.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        result.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        result.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    return result;
}

static bool decode_utf8(std::string_view input, size_t& pos, char32_t& cp) {
    if (pos >= input.size()) return false;
    auto byte = static_cast<unsigned char>(input[pos]);
    int seq_len;
    if (byte < 0x80) {
        cp = byte;
        pos += 1;
        return true;
    } else if ((byte & 0xE0) == 0xC0) {
        seq_len = 2;
        cp = byte & 0x1F;
    } else if ((byte & 0xF0) == 0xE0) {
        seq_len = 3;
        cp = byte & 0x0F;
    } else if ((byte & 0xF8) == 0xF0) {
        seq_len = 4;
        cp = byte & 0x07;
    } else {
        pos += 1;
        return false;
    }
    for (int i = 1; i < seq_len; ++i) {
        if (pos + i >= input.size()) return false;
        auto next = static_cast<unsigned char>(input[pos + i]);
        if ((next & 0xC0) != 0x80) {
            pos += i;
            return false;
        }
        cp = (cp << 6) | (next & 0x3F);
    }
    pos += seq_len;
    return true;
}

std::string normalize(std::string_view input) {
    std::string result;
    result.reserve(input.size());
    size_t pos = 0;
    char32_t cp;
    while (decode_utf8(input, pos, cp)) {
        cp = to_lower_ascii(cp);
        if (is_ascii_alnum(cp) || is_devanagari(cp)) {
            result += codepoint_to_utf8(cp);
        } else if (is_space(cp)) {
            result += ' ';
        } else {
            result += ' ';
        }
    }
    auto start = result.find_first_not_of(' ');
    if (start == std::string::npos) return "";
    auto end = result.find_last_not_of(' ');
    result = result.substr(start, end - start + 1);
    auto last = std::unique(result.begin(), result.end(),
                            [](char a, char b) { return a == ' ' && b == ' '; });
    result.erase(last, result.end());
    return result;
}

std::vector<std::string> tokenize(std::string_view normalized) {
    std::vector<std::string> tokens;
    std::string token;
    for (char c : normalized) {
        if (c == ' ') {
            if (!token.empty()) {
                tokens.push_back(std::move(token));
                token.clear();
            }
        } else {
            token += c;
        }
    }
    if (!token.empty()) {
        tokens.push_back(std::move(token));
    }
    return tokens;
}

std::string slugify(std::string_view input) {
    std::string result;
    result.reserve(input.size());
    bool last_was_hyphen = false;
    for (char c : input) {
        auto uc = static_cast<unsigned char>(c);
        if (uc >= 'A' && uc <= 'Z') {
            result.push_back(static_cast<char>(uc + 32));
            last_was_hyphen = false;
        } else if ((uc >= 'a' && uc <= 'z') || (uc >= '0' && uc <= '9')) {
            result.push_back(c);
            last_was_hyphen = false;
        } else {
            if (!last_was_hyphen) {
                result.push_back('-');
                last_was_hyphen = true;
            }
        }
    }
    auto start = result.find_first_not_of('-');
    if (start == std::string::npos) return "";
    auto end = result.find_last_not_of('-');
    return result.substr(start, end - start + 1);
}

}  // namespace engine
