#include "commands.h"
#include "../utils/args_parser.h"
#include <unigram/tokenizer.h>
#include <iostream>

void print_encode_usage() {
    std::cout << "Usage: unigram-cli encode [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --model, -m <file>     Model file (required)\n";
    std::cout << "  --text, -t <text>      Text to encode (required)\n";
    std::cout << "  --help, -h             Show this help message\n";
}

int encode_command(int argc, char* argv[]) {
    ArgsParser parser(argc, argv);

    if (parser.has_flag("--help") || parser.has_flag("-h")) {
        print_encode_usage();
        return 0;
    }

    std::string model_file = parser.get_value("--model", parser.get_value("-m", ""));
    std::string text = parser.get_value("--text", parser.get_value("-t", ""));
    
    if (model_file.empty() || text.empty()) {
        std::cerr << "Error: --model and --text are required\n\n";
        print_encode_usage();
        return 1;
    }

    // Load tokenizer
    unigram::Tokenizer tokenizer;
    if (!tokenizer.load(model_file)) {
        std::cerr << "Failed to load model from: " << model_file << "\n";
        return 1;
    }

    // Encode text
    auto token_ids = tokenizer.encode_as_ids(text);
    
    std::cout << "Token IDs: ";
    for (size_t i = 0; i < token_ids.size(); ++i) {
        std::cout << token_ids[i];
        if (i < token_ids.size() - 1) {
            std::cout << " ";
        }
    }
    std::cout << "\n";

    return 0;
}
