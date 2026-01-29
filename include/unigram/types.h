#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace unigram {

// Type aliases for clarity
using TokenId = uint32_t;
using Token = std::string;
using TokenIds = std::vector<TokenId>;
using Tokens = std::vector<Token>;
using Score = double;

// Special token IDs
constexpr TokenId UNK_TOKEN_ID = 0;
constexpr TokenId BOS_TOKEN_ID = 1;  // Begin of sequence
constexpr TokenId EOS_TOKEN_ID = 2;  // End of sequence
constexpr TokenId PAD_TOKEN_ID = 3;  // Padding

// Special tokens
inline constexpr const char* UNK_TOKEN = "<unk>";
inline constexpr const char* BOS_TOKEN = "<s>";
inline constexpr const char* EOS_TOKEN = "</s>";
inline constexpr const char* PAD_TOKEN = "<pad>";

// Configuration structure
struct TokenizerConfig {
    bool add_bos_token = false;
    bool add_eos_token = false;
    bool byte_fallback = true;
    std::optional<size_t> max_length = std::nullopt;
    
    // Training parameters
    size_t vocab_size = 32000;
    size_t num_iterations = 4;
    double shrinking_factor = 0.75;
    size_t min_frequency = 2;
    double length_penalty_alpha = 0.0;  // Length penalty factor (0 = no penalty)
};

} // namespace unigram
