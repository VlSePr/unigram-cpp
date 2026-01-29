#include <unigram/tokenizer.h>
#include <unigram/trainer.h>
#include <iostream>
#include <vector>

int main() {
    // Example: Training a simple tokenizer

    std::cout << "=== Unigram Tokenizer Training Example ===\n\n";

    // 1. Prepare training corpus
    std::vector<std::string> corpus = {
        "The quick brown fox jumps over the lazy dog",
        "Machine learning is fascinating",
        "Natural language processing is amazing",
        "Tokenization is the first step in NLP"
    };

    // 2. Configure training parameters
    unigram::TokenizerConfig config;
    config.vocab_size = 200;
    config.num_iterations = 3;
    config.min_frequency = 1;

    // 3. Create trainer and train
    unigram::Trainer trainer(config);
    
    trainer.set_progress_callback([](size_t current, size_t total, const std::string& msg) {
        std::cout << "[" << current << "/" << total << "] " << msg << "\n";
    });

    std::cout << "Training tokenizer...\n";
    auto vocab = trainer.train(corpus);

    // 4. Create tokenizer with trained vocabulary
    unigram::Tokenizer tokenizer(config);
    tokenizer.set_vocabulary(vocab);

    // 5. Test the tokenizer
    std::string test_text = "Machine learning tokenization";
    
    std::cout << "\nTesting tokenizer:\n";
    std::cout << "Input: " << test_text << "\n";
    
    auto tokens = tokenizer.encode(test_text);
    std::cout << "Tokens: ";
    for (const auto& token : tokens) {
        std::cout << "[" << token << "] ";
    }
    std::cout << "\n";

    auto token_ids = tokenizer.encode_as_ids(test_text);
    std::cout << "Token IDs: ";
    for (auto id : token_ids) {
        std::cout << id << " ";
    }
    std::cout << "\n";

    auto decoded = tokenizer.decode(tokens);
    std::cout << "Decoded: " << decoded << "\n";

    std::cout << "\nVocabulary size: " << vocab.size() << " tokens\n";

    return 0;
}
