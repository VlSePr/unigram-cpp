#include "unigram/vocabulary.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>

using json = nlohmann::json;

namespace unigram {

struct Vocabulary::Impl {
    std::vector<Token> id_to_token;
    std::vector<Score> id_to_score;
    std::unordered_map<Token, TokenId> token_to_id;
    
    Impl() {
        // Initialize with special tokens
        add_token(UNK_TOKEN, 0.0);
        add_token(BOS_TOKEN, 0.0);
        add_token(EOS_TOKEN, 0.0);
        add_token(PAD_TOKEN, 0.0);
    }
    
    TokenId add_token(const Token& token, Score score) {
        auto it = token_to_id.find(token);
        if (it != token_to_id.end()) {
            // Update score if token exists
            id_to_score[it->second] = score;
            return it->second;
        }
        
        TokenId id = static_cast<TokenId>(id_to_token.size());
        id_to_token.push_back(token);
        id_to_score.push_back(score);
        token_to_id[token] = id;
        
        return id;
    }
};

Vocabulary::Vocabulary()
    : pimpl_(std::make_unique<Impl>()) {
}

Vocabulary::~Vocabulary() = default;

Vocabulary::Vocabulary(const Vocabulary& other)
    : pimpl_(std::make_unique<Impl>(*other.pimpl_)) {
}

Vocabulary& Vocabulary::operator=(const Vocabulary& other) {
    if (this != &other) {
        pimpl_ = std::make_unique<Impl>(*other.pimpl_);
    }
    return *this;
}

Vocabulary::Vocabulary(Vocabulary&&) noexcept = default;
Vocabulary& Vocabulary::operator=(Vocabulary&&) noexcept = default;

TokenId Vocabulary::add_token(const Token& token, Score score) {
    return pimpl_->add_token(token, score);
}

TokenId Vocabulary::get_id(const Token& token) const {
    auto it = pimpl_->token_to_id.find(token);
    if (it != pimpl_->token_to_id.end()) {
        return it->second;
    }
    return UNK_TOKEN_ID;
}

const Token& Vocabulary::get_token(TokenId id) const {
    if (id >= pimpl_->id_to_token.size()) {
        return pimpl_->id_to_token[UNK_TOKEN_ID];
    }
    return pimpl_->id_to_token[id];
}

Score Vocabulary::get_score(TokenId id) const {
    if (id >= pimpl_->id_to_score.size()) {
        return 0.0;
    }
    return pimpl_->id_to_score[id];
}

size_t Vocabulary::size() const {
    return pimpl_->id_to_token.size();
}

bool Vocabulary::contains(const Token& token) const {
    return pimpl_->token_to_id.find(token) != pimpl_->token_to_id.end();
}

void Vocabulary::clear() {
    pimpl_->id_to_token.clear();
    pimpl_->id_to_score.clear();
    pimpl_->token_to_id.clear();
    
    // Re-add special tokens
    pimpl_->add_token(UNK_TOKEN, 0.0);
    pimpl_->add_token(BOS_TOKEN, 0.0);
    pimpl_->add_token(EOS_TOKEN, 0.0);
    pimpl_->add_token(PAD_TOKEN, 0.0);
}

bool Vocabulary::load(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file for reading: " << filepath << "\n";
            std::cerr << "Check that:\n";
            std::cerr << "  - The file exists\n";
            std::cerr << "  - You have read permissions\n";
            std::cerr << "  - The path is correct\n";
            return false;
        }
        
        json j;
        try {
            file >> j;
        } catch (const json::exception& e) {
            std::cerr << "Error: Invalid JSON format in file: " << filepath << "\n";
            std::cerr << "JSON error: " << e.what() << "\n";
            return false;
        }
        
        // Clear current vocabulary
        clear();
        
        // Load vocabulary entries
        if (j.contains("vocabulary") && j["vocabulary"].is_array()) {
            for (const auto& item : j["vocabulary"]) {
                if (item.contains("token") && item.contains("score")) {
                    std::string token = item["token"];
                    double score = item["score"];
                    
                    // Decode byte tokens from <0xXX> format
                    if (item.contains("is_byte") && item["is_byte"] == true) {
                        // Parse <0xXX> format
                        if (token.length() == 6 && token[0] == '<' && token[1] == '0' && 
                            token[2] == 'x' && token[5] == '>') {
                            unsigned int byte_val;
                            sscanf(token.c_str() + 3, "%02X", &byte_val);
                            token = std::string(1, static_cast<char>(byte_val));
                        }
                    }
                    
                    // Skip special tokens as they're already added in clear()
                    if (token != UNK_TOKEN && token != BOS_TOKEN && 
                        token != EOS_TOKEN && token != PAD_TOKEN) {
                        add_token(token, score);
                    }
                }
            }
        } else {
            std::cerr << "Error: Invalid vocabulary format - missing 'vocabulary' array\n";
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception while loading vocabulary: " << e.what() << "\n";
        return false;
    }
}

bool Vocabulary::save(const std::string& filepath) const {
    try {
        json j;
        
        // Metadata
        j["version"] = "1.0.0";
        j["vocab_size"] = size();
        
        // Vocabulary entries
        json vocab_array = json::array();
        for (size_t i = 0; i < pimpl_->id_to_token.size(); ++i) {
            json entry;
            entry["id"] = i;
            
            // Encode byte tokens as <0xXX> format for JSON safety
            const std::string& token = pimpl_->id_to_token[i];
            if (token.length() == 1) {
                unsigned char byte = static_cast<unsigned char>(token[0]);
                // If it's a control character or high byte, encode as hex
                if (byte < 32 || byte >= 127) {
                    char hex[8];
                    snprintf(hex, sizeof(hex), "<0x%02X>", byte);
                    entry["token"] = hex;
                    entry["is_byte"] = true;
                } else {
                    entry["token"] = token;
                }
            } else {
                entry["token"] = token;
            }
            
            entry["score"] = pimpl_->id_to_score[i];
            vocab_array.push_back(entry);
        }
        j["vocabulary"] = vocab_array;
        
        // Write to file
        std::ofstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file for writing: " << filepath << "\n";
            std::cerr << "Check that:\n";
            std::cerr << "  - The directory exists\n";
            std::cerr << "  - You have write permissions\n";
            std::cerr << "  - The path is not a directory\n";
            return false;
        }
        
        file << j.dump(2);  // Pretty print with 2-space indent
        
        if (!file.good()) {
            std::cerr << "Error: Failed to write data to file: " << filepath << "\n";
            std::cerr << "The file may be locked or disk may be full\n";
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception while saving vocabulary: " << e.what() << "\n";
        return false;
    }
}

} // namespace unigram
