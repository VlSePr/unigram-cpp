#pragma once

#include <string>
#include <vector>

namespace unigram::internal {

/**
 * @brief UTF-8 text processing utilities
 */
class UTF8Utils {
public:
    /**
     * @brief Split text into UTF-8 characters
     * @param text Input text
     * @return Vector of UTF-8 character strings
     */
    static std::vector<std::string> split_utf8(const std::string& text);
    
    /**
     * @brief Check if byte is a UTF-8 continuation byte
     * @param byte The byte to check
     * @return true if continuation byte
     */
    static bool is_continuation_byte(unsigned char byte);
    
    /**
     * @brief Normalize whitespace in text
     * @param text Input text
     * @return Normalized text
     */
    static std::string normalize_whitespace(const std::string& text);
};

} // namespace unigram::internal
