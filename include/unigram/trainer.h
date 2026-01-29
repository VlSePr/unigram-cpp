#pragma once

#include "types.h"
#include "vocabulary.h"
#include <string>
#include <memory>
#include <functional>

namespace unigram {

/**
 * @brief Progress callback for training
 * @param current Current iteration
 * @param total Total iterations
 * @param message Status message
 */
using ProgressCallback = std::function<void(size_t current, size_t total, const std::string& message)>;

/**
 * @brief Trainer for Unigram tokenizer
 * 
 * Implements the EM algorithm for training a Unigram language model
 * from a text corpus.
 */
class Trainer {
public:
    /**
     * @brief Construct a new Trainer
     * @param config Training configuration
     */
    explicit Trainer(const TokenizerConfig& config = TokenizerConfig{});
    ~Trainer();

    /**
     * @brief Train a Unigram model from text corpus
     * @param corpus Vector of text samples
     * @return Trained vocabulary
     */
    Vocabulary train(const std::vector<std::string>& corpus);

    /**
     * @brief Train from a file containing text (one sample per line)
     * @param filepath Path to corpus file
     * @return Trained vocabulary
     */
    Vocabulary train_from_file(const std::string& filepath);

    /**
     * @brief Train from multiple files
     * @param filepaths Vector of file paths to process
     * @return Trained vocabulary
     */
    Vocabulary train_from_files(const std::vector<std::string>& filepaths);

    /**
     * @brief Train from all text files in a directory
     * @param directory_path Path to directory containing text files
     * @param pattern File pattern to match (default: "*" - all files)
     * @return Trained vocabulary
     */
    Vocabulary train_from_directory(const std::string& directory_path, 
                                    const std::string& pattern = "*");

    /**
     * @brief Set progress callback for training
     * @param callback Callback function
     */
    void set_progress_callback(ProgressCallback callback);

    /**
     * @brief Get training configuration
     * @return Current configuration
     */
    const TokenizerConfig& config() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace unigram
