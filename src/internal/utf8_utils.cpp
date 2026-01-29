#include "utf8_utils.h"
#include <algorithm>
#include <cctype>

namespace unigram::internal {

std::vector<std::string> UTF8Utils::split_utf8(const std::string& text) {
    std::vector<std::string> result;
    
    for (size_t i = 0; i < text.size();) {
        unsigned char c = text[i];
        size_t char_len = 1;
        
        // Determine UTF-8 character length
        if ((c & 0x80) == 0) {
            char_len = 1;  // ASCII
        } else if ((c & 0xE0) == 0xC0) {
            char_len = 2;  // 2-byte UTF-8
        } else if ((c & 0xF0) == 0xE0) {
            char_len = 3;  // 3-byte UTF-8
        } else if ((c & 0xF8) == 0xF0) {
            char_len = 4;  // 4-byte UTF-8
        }
        
        if (i + char_len <= text.size()) {
            result.push_back(text.substr(i, char_len));
        }
        
        i += char_len;
    }
    
    return result;
}

bool UTF8Utils::is_continuation_byte(unsigned char byte) {
    return (byte & 0xC0) == 0x80;
}

std::string UTF8Utils::normalize_whitespace(const std::string& text) {
    std::string result;
    bool prev_space = false;
    
    for (char c : text) {
        if (std::isspace(c)) {
            if (!prev_space && !result.empty()) {
                result += ' ';
                prev_space = true;
            }
        } else {
            result += c;
            prev_space = false;
        }
    }
    
    return result;
}

} // namespace unigram::internal
