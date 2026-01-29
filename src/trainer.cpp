#include "unigram/trainer.h"
#include "internal/utf8_utils.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <filesystem>
#include <queue>
#include <cmath>
#include <iostream>
#include <mutex>
#ifdef USE_OPENMP
#include <omp.h>
#endif
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

namespace unigram {

struct Trainer::Impl {
    TokenizerConfig config;
    ProgressCallback progress_callback;
    
    explicit Impl(const TokenizerConfig& cfg) : config(cfg) {}
    
    // Count substrings in corpus with their frequencies
    // Uses UTF-8 character boundaries to avoid splitting multi-byte characters
    std::unordered_map<std::string, size_t> count_substrings(
        const std::vector<std::string>& corpus, size_t max_length = 10) {
        
        std::unordered_map<std::string, size_t> freq;
        
#ifdef USE_OPENMP
        // Parallel processing with thread-local frequency maps
        std::mutex freq_mutex;
        #pragma omp parallel
        {
            std::unordered_map<std::string, size_t> local_freq;
            
            #pragma omp for nowait
            for (int idx = 0; idx < static_cast<int>(corpus.size()); ++idx) {
                const auto& text = corpus[idx];
                // Split text into UTF-8 characters
                auto utf8_chars = internal::UTF8Utils::split_utf8(text);
                
                // Count all valid UTF-8 substrings up to max_length characters
                for (size_t i = 0; i < utf8_chars.size(); ++i) {
                    std::string substr;
                    for (size_t len = 1; len <= std::min(max_length, utf8_chars.size() - i); ++len) {
                        substr += utf8_chars[i + len - 1];
                        local_freq[substr]++;
                    }
                }
            }
            
            // Merge local results into global frequency map
            std::lock_guard<std::mutex> lock(freq_mutex);
            for (const auto& [token, count] : local_freq) {
                freq[token] += count;
            }
        }
#else
        // Sequential fallback
        for (const auto& text : corpus) {
            // Split text into UTF-8 characters
            auto utf8_chars = internal::UTF8Utils::split_utf8(text);
            
            // Count all valid UTF-8 substrings up to max_length characters
            for (size_t i = 0; i < utf8_chars.size(); ++i) {
                std::string substr;
                for (size_t len = 1; len <= std::min(max_length, utf8_chars.size() - i); ++len) {
                    substr += utf8_chars[i + len - 1];
                    freq[substr]++;
                }
            }
        }
#endif
        
        return freq;
    }
    
    // Count words (whitespace-delimited tokens)
    std::unordered_map<std::string, size_t> count_words(
        const std::vector<std::string>& corpus) {
        
        std::unordered_map<std::string, size_t> word_freq;
        
        for (const auto& text : corpus) {
            std::string word;
            for (char c : text) {
                if (std::isspace(c)) {
                    if (!word.empty()) {
                        word_freq[word]++;
                        word.clear();
                    }
                } else {
                    word += c;
                }
            }
            if (!word.empty()) {
                word_freq[word]++;
            }
        }
        
        return word_freq;
    }
    
    // Initialize vocabulary with word-focused approach
    Vocabulary initialize_vocabulary(
        const std::vector<std::string>& corpus,
        size_t initial_vocab_size) {
        
        Vocabulary vocab;
                // Step 0: Add all byte-level tokens as fallback (0x00 to 0xFF)
        // This ensures we can always encode any input, even with unknown characters
        for (int byte = 0; byte < 256; ++byte) {
            std::string byte_token(1, static_cast<char>(byte));
            // Give bytes a low score so they're used as fallback only
            vocab.add_token(byte_token, -15.0);
        }
                // Step 1: Count complete words (whitespace-delimited)
        auto word_freq = count_words(corpus);
        
        // Step 2: Count all substrings (for subword units)
        auto substr_freq = count_substrings(corpus);
        
        // Step 3: Boost scores for complete words (3x multiplier)
        for (const auto& [word, count] : word_freq) {
            if (substr_freq.find(word) != substr_freq.end()) {
                substr_freq[word] = substr_freq[word] * 3;  // Boost complete words
            }
        }
        
        // Step 4: Penalize very short tokens (1-2 chars) unless they're complete words
        for (auto& [token, count] : substr_freq) {
            size_t char_count = internal::UTF8Utils::split_utf8(token).size();
            if (char_count <= 2 && word_freq.find(token) == word_freq.end()) {
                count = count / 2;  // Penalize short non-word tokens
            }
        }
        
        // Step 5: Sort by frequency
        std::vector<std::pair<std::string, size_t>> freq_pairs(
            substr_freq.begin(), substr_freq.end());
        
        std::sort(freq_pairs.begin(), freq_pairs.end(),
            [](const auto& a, const auto& b) {
                return a.second > b.second;  // Descending by frequency
            });
        
        // Step 6: Add top frequent tokens to vocabulary
        size_t added = 0;
        size_t total_count = 0;
        for (const auto& [_, count] : freq_pairs) {
            total_count += count;
        }
        
        for (const auto& [token, count] : freq_pairs) {
            if (added >= initial_vocab_size) break;
            if (count < config.min_frequency) continue;
            
            // Score is log probability
            double score = std::log(static_cast<double>(count) / total_count);
            vocab.add_token(token, score);
            ++added;
        }
        
        return vocab;
    }
    
    // Calculate expected counts using Viterbi decoding
    std::unordered_map<std::string, double> calculate_expected_counts(
        const std::vector<std::string>& corpus,
        const Vocabulary& vocab) {
        
        std::unordered_map<std::string, double> expected_counts;
        
#ifdef USE_OPENMP
        // Parallel processing - each thread processes different sentences
        std::mutex counts_mutex;
        
        #pragma omp parallel
        {
            std::unordered_map<std::string, double> local_counts;
            
            #pragma omp for nowait schedule(dynamic, 100)
            for (int idx = 0; idx < static_cast<int>(corpus.size()); ++idx) {
                const auto& text = corpus[idx];
                if (text.empty()) continue;
                
                const size_t len = text.length();
                
                // Viterbi forward pass
                std::vector<double> best_score(len + 1, -std::numeric_limits<double>::infinity());
                std::vector<size_t> backtrack(len + 1, 0);
                best_score[0] = 0.0;
                
                for (size_t end = 1; end <= len; ++end) {
                    for (size_t start = 0; start < end; ++start) {
                        std::string token = text.substr(start, end - start);
                        
                        if (!vocab.contains(token)) continue;
                        
                        double token_score = vocab.get_score(vocab.get_id(token));
                        double total_score = best_score[start] + token_score;
                        
                        if (total_score > best_score[end]) {
                            best_score[end] = total_score;
                            backtrack[end] = start;
                        }
                    }
                }
                
                // Backward pass: collect tokens used
                size_t pos = len;
                while (pos > 0) {
                    size_t start = backtrack[pos];
                    std::string token = text.substr(start, pos - start);
                    local_counts[token] += 1.0;
                    pos = start;
                }
            }
            
            // Merge local counts into global map
            std::lock_guard<std::mutex> lock(counts_mutex);
            for (const auto& [token, count] : local_counts) {
                expected_counts[token] += count;
            }
        }
#else
        // Sequential fallback
        for (const auto& text : corpus) {
            if (text.empty()) continue;
            
            const size_t len = text.length();
            
            // Viterbi forward pass
            std::vector<double> best_score(len + 1, -std::numeric_limits<double>::infinity());
            std::vector<size_t> backtrack(len + 1, 0);
            best_score[0] = 0.0;
            
            for (size_t end = 1; end <= len; ++end) {
                for (size_t start = 0; start < end; ++start) {
                    std::string token = text.substr(start, end - start);
                    
                    if (!vocab.contains(token)) continue;
                    
                    double token_score = vocab.get_score(vocab.get_id(token));
                    double total_score = best_score[start] + token_score;
                    
                    if (total_score > best_score[end]) {
                        best_score[end] = total_score;
                        backtrack[end] = start;
                    }
                }
            }
            
            // Backward pass: collect tokens used
            size_t pos = len;
            while (pos > 0) {
                size_t start = backtrack[pos];
                std::string token = text.substr(start, pos - start);
                expected_counts[token] += 1.0;
                pos = start;
            }
        }
#endif
        
        return expected_counts;
    }
    
    // Prune vocabulary to target size, keeping highest scoring tokens
    Vocabulary prune_vocabulary(
        const Vocabulary& vocab,
        const std::unordered_map<std::string, double>& expected_counts,
        size_t target_size) {
        
        // First, preserve all byte-level tokens (256 single-byte tokens 0x00-0xFF)
        // These must be kept for fallback even if they don't appear in corpus
        std::vector<std::pair<std::string, double>> byte_tokens;
        std::vector<std::pair<std::string, double>> token_scores;
        
        double total_count = 0.0;
        for (const auto& [_, count] : expected_counts) {
            total_count += count;
        }
        
        for (size_t i = 0; i < vocab.size(); ++i) {
            const auto& token = vocab.get_token(i);
            auto it = expected_counts.find(token);
            
            // Check if this is a byte token (single byte with any value)
            bool is_byte_token = (token.length() == 1);
            
            if (is_byte_token) {
                // Preserve byte tokens with their original low score
                double score = vocab.get_score(i);
                byte_tokens.emplace_back(token, score);
            } else if (it != expected_counts.end() && it->second > 0) {
                // Regular tokens: calculate score from expected counts
                // Apply length penalty: score = logP(token) - alpha * length
                double log_prob = std::log(it->second / total_count);
                double length_penalty = config.length_penalty_alpha * token.length();
                double score = log_prob - length_penalty;
                token_scores.emplace_back(token, score);
            }
        }
        
        // Sort regular tokens by score (descending)
        std::sort(token_scores.begin(), token_scores.end(),
            [](const auto& a, const auto& b) {
                return a.second > b.second;
            });
        
        // Create new vocabulary: byte tokens + top regular tokens
        Vocabulary new_vocab;
        
        // First, add all byte tokens (should be 256)
        for (const auto& [token, score] : byte_tokens) {
            new_vocab.add_token(token, score);
        }
        
        // Then add regular tokens up to target size
        size_t kept = byte_tokens.size();
        for (const auto& [token, score] : token_scores) {
            if (kept >= target_size) break;
            new_vocab.add_token(token, score);
            ++kept;
        }
        
        return new_vocab;
    }
    
    void report_progress(size_t current, size_t total, const std::string& msg) {
        if (progress_callback) {
            progress_callback(current, total, msg);
        }
    }
};

Trainer::Trainer(const TokenizerConfig& config)
    : pimpl_(std::make_unique<Impl>(config)) {
}

Trainer::~Trainer() = default;

Vocabulary Trainer::train(const std::vector<std::string>& corpus) {
    if (corpus.empty()) {
        throw std::invalid_argument("Corpus cannot be empty");
    }
    
    pimpl_->report_progress(0, pimpl_->config.num_iterations + 1, "Starting training - analyzing corpus...");
    
    // Initialize with most frequent substrings (seed vocab is larger than target)
    size_t initial_size = pimpl_->config.vocab_size * 3;
    Vocabulary vocab = pimpl_->initialize_vocabulary(corpus, initial_size);
    
    pimpl_->report_progress(0, pimpl_->config.num_iterations + 1, 
        "Initialized vocabulary with " + std::to_string(vocab.size()) + " tokens");
    
    // EM algorithm iterations
    for (size_t iter = 0; iter < pimpl_->config.num_iterations; ++iter) {
        // Report E-step
        std::string msg = "Iteration " + std::to_string(iter + 1) + "/" +
                         std::to_string(pimpl_->config.num_iterations) + " - E-step: calculating expected counts";
        pimpl_->report_progress(iter + 1, pimpl_->config.num_iterations + 1, msg);
        
        // E-step: Calculate expected counts
        auto expected_counts = pimpl_->calculate_expected_counts(corpus, vocab);
        
        // Report M-step
        msg = "Iteration " + std::to_string(iter + 1) + "/" +
              std::to_string(pimpl_->config.num_iterations) + " - M-step: updating probabilities";
        pimpl_->report_progress(iter + 1, pimpl_->config.num_iterations + 1, msg);
        
        // M-step: Update probabilities and prune
        vocab = pimpl_->prune_vocabulary(vocab, expected_counts, pimpl_->config.vocab_size);
    }
    
    pimpl_->report_progress(pimpl_->config.num_iterations + 1, 
                           pimpl_->config.num_iterations + 1, 
                           "Training complete! Final vocabulary: " + std::to_string(vocab.size()) + " tokens");
    
    return vocab;
}

Vocabulary Trainer::train_from_file(const std::string& filepath) {
    // Read file as binary to preserve UTF-8 encoding
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    
    std::vector<std::string> corpus;
    std::string line;
    
    pimpl_->report_progress(0, 1, "Loading corpus from: " + filepath);
    
    while (std::getline(file, line)) {
        // Remove carriage return if present (Windows line endings)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (!line.empty()) {
            corpus.push_back(line);
        }
    }
    
    pimpl_->report_progress(0, 1, "Loaded " + std::to_string(corpus.size()) + " lines");
    
    return train(corpus);
}

Vocabulary Trainer::train_from_files(const std::vector<std::string>& filepaths) {
    if (filepaths.empty()) {
        throw std::invalid_argument("File list cannot be empty");
    }
    
    std::vector<std::string> corpus;
    size_t total_files = filepaths.size();
    
    for (size_t i = 0; i < filepaths.size(); ++i) {
        const auto& filepath = filepaths[i];
        
        std::string msg = "Loading file " + std::to_string(i + 1) + "/" + 
                         std::to_string(total_files) + ": " + filepath;
        pimpl_->report_progress(i, total_files, msg);
        
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Warning: Failed to open file: " << filepath << ", skipping...\n";
            continue;
        }
        
        std::string line;
        size_t line_count = 0;
        while (std::getline(file, line)) {
            // Remove carriage return if present (Windows line endings)
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (!line.empty()) {
                corpus.push_back(line);
                ++line_count;
            }
        }
        
        std::cout << "  Loaded " << line_count << " lines from " << filepath << "\n";
    }
    
    if (corpus.empty()) {
        throw std::runtime_error("No data loaded from any file");
    }
    
    std::string msg = "Total corpus: " + std::to_string(corpus.size()) + " lines from " + 
                     std::to_string(total_files) + " files";
    pimpl_->report_progress(total_files, total_files, msg);
    
    return train(corpus);
}

Vocabulary Trainer::train_from_directory(const std::string& directory_path, 
                                         const std::string& pattern) {
    namespace fs = std::filesystem;
    
    if (!fs::exists(directory_path)) {
        throw std::runtime_error("Directory does not exist: " + directory_path);
    }
    
    if (!fs::is_directory(directory_path)) {
        throw std::runtime_error("Path is not a directory: " + directory_path);
    }
    
    std::vector<std::string> filepaths;
    
    // Collect all matching files
    pimpl_->report_progress(0, 1, "Scanning directory: " + directory_path);
    
    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            std::string extension = entry.path().extension().string();
            
            // Pattern matching logic
            if (pattern == "*" || pattern == "*.*") {
                // All files
                filepaths.push_back(entry.path().string());
            } else if (pattern.find("*.") == 0) {
                // Extension pattern like *.txt
                std::string ext = pattern.substr(1);  // Remove the *
                if (extension == ext) {
                    filepaths.push_back(entry.path().string());
                }
            } else if (filename.find(pattern) != std::string::npos) {
                // Substring match
                filepaths.push_back(entry.path().string());
            }
        }
    }
    
    if (filepaths.empty()) {
        throw std::runtime_error("No files matching pattern '" + pattern + 
                               "' found in directory: " + directory_path);
    }
    
    std::cout << "Found " << filepaths.size() << " files matching pattern '" 
              << pattern << "'\n";
    
    // Sort for consistent processing order
    std::sort(filepaths.begin(), filepaths.end());
    
    return train_from_files(filepaths);
}

void Trainer::set_progress_callback(ProgressCallback callback) {
    pimpl_->progress_callback = std::move(callback);
}

const TokenizerConfig& Trainer::config() const {
    return pimpl_->config;
}

} // namespace unigram
