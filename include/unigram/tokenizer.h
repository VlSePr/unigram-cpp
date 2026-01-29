#pragma once

#include "types.h"
#include "vocabulary.h"
#include <string>
#include <memory>

namespace unigram {

/**
 * @brief Unigram tokenizer implementation
 * 
 * Implements the Unigram language model based tokenization algorithm.
 * Uses dynamic programming to find the most probable segmentation.
 */
class Tokenizer {
public:
    /**
     * @brief Construct a new Tokenizer
     * @param config Optional configuration
     */
    explicit Tokenizer(const TokenizerConfig& config = TokenizerConfig{});
    ~Tokenizer();

    // Prevent copying, allow moving
    Tokenizer(const Tokenizer&) = delete;
    Tokenizer& operator=(const Tokenizer&) = delete;
    Tokenizer(Tokenizer&&) noexcept;
    Tokenizer& operator=(Tokenizer&&) noexcept;

    /**
     * @brief Set vocabulary for the tokenizer
     * @param vocab The vocabulary to use
     */
    void set_vocabulary(const Vocabulary& vocab);

    /**
     * @brief Encode text into tokens
     * @param text Input text
     * @return Vector of token strings
     */
    Tokens encode(const std::string& text) const;

    /**
     * @brief Encode text into token IDs
     * @param text Input text
     * @return Vector of token IDs
     */
    TokenIds encode_as_ids(const std::string& text) const;

    /**
     * @brief Decode tokens back to text
     * @param tokens Vector of token strings
     * @return Decoded text
     */
    std::string decode(const Tokens& tokens) const;

    /**
     * @brief Decode token IDs back to text
     * @param token_ids Vector of token IDs
     * @return Decoded text
     */
    std::string decode(const TokenIds& token_ids) const;

    /**
     * @brief Load tokenizer model from file
     * @param filepath Path to model file (JSON format)
     * @return true if successful
     */
    bool load(const std::string& filepath);

    /**
     * @brief Save tokenizer model to file
     * @param filepath Path to model file (JSON format)
     * @return true if successful
     */
    bool save(const std::string& filepath) const;

    /**
     * @brief Get the current configuration
     * @return Current tokenizer configuration
     */
    const TokenizerConfig& config() const;

    /**
     * @brief Get the vocabulary
     * @return Reference to the vocabulary
     */
    const Vocabulary& vocabulary() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace unigram
