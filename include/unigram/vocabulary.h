#pragma once

#include "types.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace unigram {

/**
 * @brief Vocabulary manager for the Unigram tokenizer
 * 
 * Manages the mapping between tokens and their IDs, along with scores
 * used in the Unigram language model for tokenization.
 */
class Vocabulary {
public:
    Vocabulary();
    ~Vocabulary();
    
    // Copy constructor and assignment
    Vocabulary(const Vocabulary& other);
    Vocabulary& operator=(const Vocabulary& other);
    
    // Move constructor and assignment
    Vocabulary(Vocabulary&&) noexcept;
    Vocabulary& operator=(Vocabulary&&) noexcept;

    /**
     * @brief Add a token to the vocabulary
     * @param token The token string
     * @param score The score/probability of the token
     * @return The assigned token ID
     */
    TokenId add_token(const Token& token, Score score);

    /**
     * @brief Get token ID by token string
     * @param token The token string
     * @return Token ID, or UNK_TOKEN_ID if not found
     */
    TokenId get_id(const Token& token) const;

    /**
     * @brief Get token string by token ID
     * @param id The token ID
     * @return Token string
     */
    const Token& get_token(TokenId id) const;

    /**
     * @brief Get score of a token
     * @param id The token ID
     * @return Token score
     */
    Score get_score(TokenId id) const;

    /**
     * @brief Get vocabulary size
     * @return Number of tokens in vocabulary
     */
    size_t size() const;

    /**
     * @brief Check if token exists in vocabulary
     * @param token The token string
     * @return true if token exists
     */
    bool contains(const Token& token) const;

    /**
     * @brief Clear all tokens from vocabulary
     */
    void clear();

    /**
     * @brief Load vocabulary from JSON file
     * @param filepath Path to JSON file
     * @return true if successful
     */
    bool load(const std::string& filepath);

    /**
     * @brief Save vocabulary to JSON file
     * @param filepath Path to JSON file
     * @return true if successful
     */
    bool save(const std::string& filepath) const;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace unigram
