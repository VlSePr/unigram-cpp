#include <unigram/tokenizer.h>
#include <iostream>

int main() {
    // Example: Basic usage of the tokenizer

    std::cout << "=== Unigram Tokenizer Basic Usage ===\n\n";

    // Create tokenizer with default configuration
    unigram::TokenizerConfig config;
    config.add_bos_token = true;
    config.add_eos_token = true;

    unigram::Tokenizer tokenizer(config);

    // In a real scenario, you would load a trained model:
    // tokenizer.load("model.json");

    // Encode text to tokens
    std::string text = "Hello, world! This is a test.";
    std::cout << "Original text: " << text << "\n\n";

    auto tokens = tokenizer.encode(text);
    std::cout << "Tokens:\n";
    for (size_t i = 0; i < tokens.size(); ++i) {
        std::cout << "  [" << i << "] " << tokens[i] << "\n";
    }

    // Encode text to IDs
    auto token_ids = tokenizer.encode_as_ids(text);
    std::cout << "\nToken IDs: ";
    for (auto id : token_ids) {
        std::cout << id << " ";
    }
    std::cout << "\n";

    // Decode back to text
    auto decoded_text = tokenizer.decode(tokens);
    std::cout << "\nDecoded text: " << decoded_text << "\n";

    // Decode from IDs
    auto decoded_from_ids = tokenizer.decode(token_ids);
    std::cout << "Decoded from IDs: " << decoded_from_ids << "\n";

    return 0;
}
