#include "unigram/tokenizer.h"
#include "internal/utf8_utils.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace unigram {

// Implementation details using PIMPL pattern
struct Tokenizer::Impl {
    TokenizerConfig config;
    Vocabulary vocab;
    
    explicit Impl(const TokenizerConfig& cfg) : config(cfg) {}
    
    // Viterbi algorithm for finding optimal tokenization
    Tokens viterbi_encode(const std::string& text) const {
        if (text.empty()) {
            return {};
        }
        
        const size_t len = text.length();
        
        // Dynamic programming arrays
        // best_score[i] = best log probability for text[0...i)
        std::vector<double> best_score(len + 1, -std::numeric_limits<double>::infinity());
        // backtrack[i] = start position of the token that ends at position i
        std::vector<size_t> backtrack(len + 1, 0);
        
        best_score[0] = 0.0;  // Empty prefix has score 0
        
        // Forward pass: compute best scores
        for (size_t end = 1; end <= len; ++end) {
            // Try all possible tokens ending at position 'end'
            for (size_t start = 0; start < end; ++start) {
                // Extract substring
                std::string token = text.substr(start, end - start);
                
                // Get token score (log probability)
                double token_score;
                if (vocab.contains(token)) {
                    token_score = vocab.get_score(vocab.get_id(token));
                } else {
                    // Unknown token penalty: assume very low probability
                    // Use a very negative score to strongly prefer known tokens
                    token_score = -100.0 * static_cast<double>(token.length());
                }
                
                // Total score = previous best + current token score
                double total_score = best_score[start] + token_score;
                
                // Update if this is better
                if (total_score > best_score[end]) {
                    best_score[end] = total_score;
                    backtrack[end] = start;
                }
            }
        }
        
        // Backward pass: reconstruct best tokenization
        Tokens result;
        size_t pos = len;
        
        while (pos > 0) {
            size_t start = backtrack[pos];
            std::string token = text.substr(start, pos - start);
            result.push_back(token);
            pos = start;
        }
        
        // Reverse to get correct order
        std::reverse(result.begin(), result.end());
        
        // Replace unknown tokens with UNK token
        for (auto& token : result) {
            if (!vocab.contains(token)) {
                token = UNK_TOKEN;
            }
        }
        
        return result;
    }
    
    TokenIds tokens_to_ids(const Tokens& tokens) const {
        TokenIds ids;
        ids.reserve(tokens.size());
        
        for (const auto& token : tokens) {
            ids.push_back(vocab.get_id(token));
        }
        
        return ids;
    }
    
    std::string ids_to_text(const TokenIds& ids) const {
        std::string result;
        
        for (TokenId id : ids) {
            if (!result.empty() && id != UNK_TOKEN_ID) {
                result += " ";
            }
            result += vocab.get_token(id);
        }
        
        return result;
    }
};

Tokenizer::Tokenizer(const TokenizerConfig& config)
    : pimpl_(std::make_unique<Impl>(config)) {
}

Tokenizer::~Tokenizer() = default;

Tokenizer::Tokenizer(Tokenizer&&) noexcept = default;
Tokenizer& Tokenizer::operator=(Tokenizer&&) noexcept = default;

void Tokenizer::set_vocabulary(const Vocabulary& vocab) {
    pimpl_->vocab = vocab;
}

Tokens Tokenizer::encode(const std::string& text) const {
    Tokens result = pimpl_->viterbi_encode(text);
    
    if (pimpl_->config.add_bos_token) {
        result.insert(result.begin(), BOS_TOKEN);
    }
    if (pimpl_->config.add_eos_token) {
        result.push_back(EOS_TOKEN);
    }
    
    return result;
}

TokenIds Tokenizer::encode_as_ids(const std::string& text) const {
    auto tokens = encode(text);
    return pimpl_->tokens_to_ids(tokens);
}

std::string Tokenizer::decode(const Tokens& tokens) const {
    std::string result;
    
    for (const auto& token : tokens) {
        // Skip special tokens in decoding
        if (token == BOS_TOKEN || token == EOS_TOKEN || token == PAD_TOKEN) {
            continue;
        }
        
        if (!result.empty()) {
            result += " ";
        }
        result += token;
    }
    
    return result;
}

std::string Tokenizer::decode(const TokenIds& token_ids) const {
    return pimpl_->ids_to_text(token_ids);
}

bool Tokenizer::load(const std::string& filepath) {
    return pimpl_->vocab.load(filepath);
}

bool Tokenizer::save(const std::string& filepath) const {
    return pimpl_->vocab.save(filepath);
}

const TokenizerConfig& Tokenizer::config() const {
    return pimpl_->config;
}

const Vocabulary& Tokenizer::vocabulary() const {
    return pimpl_->vocab;
}

} // namespace unigram
