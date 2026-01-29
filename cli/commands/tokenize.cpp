#include "commands.h"
#include "../utils/args_parser.h"
#include <unigram/tokenizer.h>
#include <iostream>
#include <fstream>
#include <sstream>

void print_tokenize_usage() {
    std::cout << "Usage: unigram-cli tokenize [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --model, -m <file>     Model file (required)\n";
    std::cout << "  --text, -t <string>    Text to tokenize (optional)\n";
    std::cout << "  --input, -i <file>     Input file (optional, uses stdin if not provided)\n";
    std::cout << "  --output, -o <file>    Output file (optional, uses stdout if not provided)\n";
    std::cout << "  --help, -h             Show this help message\n";
}

int tokenize_command(int argc, char* argv[]) {
    ArgsParser parser(argc, argv);

    if (parser.has_flag("--help") || parser.has_flag("-h")) {
        print_tokenize_usage();
        return 0;
    }

    std::string model_file = parser.get_value("--model", parser.get_value("-m", ""));
    
    if (model_file.empty()) {
        std::cerr << "Error: --model is required\n\n";
        print_tokenize_usage();
        return 1;
    }

    // Load tokenizer
    unigram::Tokenizer tokenizer;
    if (!tokenizer.load(model_file)) {
        std::cerr << "Failed to load model from: " << model_file << "\n";
        return 1;
    }

    std::string text_arg = parser.get_value("--text", parser.get_value("-t", ""));
    std::string input_file = parser.get_value("--input", parser.get_value("-i", ""));
    std::string output_file = parser.get_value("--output", parser.get_value("-o", ""));

    // If --text is provided, tokenize it directly
    if (!text_arg.empty()) {
        auto tokens = tokenizer.encode(text_arg);
        for (size_t i = 0; i < tokens.size(); ++i) {
            std::cout << tokens[i];
            if (i < tokens.size() - 1) {
                std::cout << " ";
            }
        }
        std::cout << "\n";
        return 0;
    }

    // Setup input stream
    std::unique_ptr<std::istream> input;
    std::ifstream file_input;
    
    if (!input_file.empty()) {
        file_input.open(input_file);
        if (!file_input.is_open()) {
            std::cerr << "Failed to open input file: " << input_file << "\n";
            return 1;
        }
        input.reset(&file_input);
    } else {
        input.reset(&std::cin);
    }

    // Setup output stream
    std::unique_ptr<std::ostream> output;
    std::ofstream file_output;
    
    if (!output_file.empty()) {
        file_output.open(output_file);
        if (!file_output.is_open()) {
            std::cerr << "Failed to open output file: " << output_file << "\n";
            return 1;
        }
        output.reset(&file_output);
    } else {
        output.reset(&std::cout);
    }

    // Process input line by line
    std::string line;
    while (std::getline(*input, line)) {
        auto tokens = tokenizer.encode(line);
        
        for (size_t i = 0; i < tokens.size(); ++i) {
            *output << tokens[i];
            if (i < tokens.size() - 1) {
                *output << " ";
            }
        }
        *output << "\n";
    }

    return 0;
}
