#include "commands.h"
#include "../utils/args_parser.h"
#include <unigram/trainer.h>
#include <unigram/tokenizer.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <filesystem>

void print_train_usage() {
    std::cout << "Usage: unigram-cli train [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --input, -i <file>      Input corpus file\n";
    std::cout << "  --input-dir <dir>       Input directory (processes all files)\n";
    std::cout << "  --pattern <pattern>     File pattern for --input-dir (default: * - all files)\n";
    std::cout << "  --output, -o <file>     Output model file (default: model/tokenizer_<vocabsize>.json)\n";
    std::cout << "  --model, -m <file>      Alias for --output\n";
    std::cout << "  --vocab-size <size>     Vocabulary size (default: 32000)\n";
    std::cout << "  --iterations <num>      Number of training iterations (default: 4)\n";
    std::cout << "  --min-frequency <num>   Minimum token frequency (default: 2)\n";
    std::cout << "  --length-penalty <val>  Length penalty factor (default: 0.0, try 0.1-0.3)\n";
    std::cout << "  --verbose, -v           Enable verbose output\n";
    std::cout << "  --help, -h              Show this help message\n\n";
    std::cout << "Examples:\n";
    std::cout << "  Single file:   unigram-cli train -i corpus.txt -o model.json\n";
    std::cout << "  Auto output:   unigram-cli train --input-dir data\n";
    std::cout << "  Directory:     unigram-cli train --input-dir data -o model.json\n";
    std::cout << "  With pattern:  unigram-cli train --input-dir data --pattern *.txt -o model.json\n";
}

int train_command(int argc, char* argv[]) {
    ArgsParser parser(argc, argv);

    if (parser.has_flag("--help") || parser.has_flag("-h")) {
        print_train_usage();
        return 0;
    }

    std::string input_file = parser.get_value("--input", parser.get_value("-i", ""));
    std::string input_dir = parser.get_value("--input-dir", "");
    std::string pattern = parser.get_value("--pattern", "*");
    std::string output_file = parser.get_value("--output", parser.get_value("-o", parser.get_value("--model", parser.get_value("-m", ""))));
    bool verbose = parser.has_flag("--verbose") || parser.has_flag("-v");
    
    // Parse training parameters
    unigram::TokenizerConfig config;
    config.vocab_size = parser.get_int("--vocab-size", 32000);
    config.num_iterations = parser.get_int("--iterations", 4);
    config.min_frequency = parser.get_int("--min-frequency", 2);
    config.length_penalty_alpha = parser.get_double("--length-penalty", 0.0);
    
    // Check that we have either input file or input directory
    if (input_file.empty() && input_dir.empty()) {
        std::cerr << "Error: Either --input or --input-dir is required\n\n";
        print_train_usage();
        return 1;
    }
    
    if (!input_file.empty() && !input_dir.empty()) {
        std::cerr << "Error: Cannot specify both --input and --input-dir\n\n";
        print_train_usage();
        return 1;
    }
    
    // Set default output file if not specified
    if (output_file.empty()) {
        // Create default model directory
        std::filesystem::path model_dir = "model";
        
        try {
            if (!std::filesystem::exists(model_dir)) {
                std::filesystem::create_directories(model_dir);
                if (verbose) {
                    std::cout << "Created model directory: " << model_dir << "\n";
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Warning: Failed to create model directory: " << e.what() << "\n";
            std::cerr << "Saving to current directory instead.\n";
            model_dir = ".";
        }
        
        // Generate default filename based on vocab size
        output_file = (model_dir / ("tokenizer_" + std::to_string(config.vocab_size) + ".json")).string();
        
        if (verbose) {
            std::cout << "Using default output: " << output_file << "\n\n";
        }
    }

    std::cout << "Training tokenizer...\n";
    if (!input_file.empty()) {
        std::cout << "Input file: " << input_file << "\n";
    } else {
        std::cout << "Input directory: " << input_dir << "\n";
        std::cout << "File pattern: " << pattern << "\n";
    }
    std::cout << "Output: " << output_file << "\n";
    std::cout << "Vocab size: " << config.vocab_size << "\n";
    std::cout << "Iterations: " << config.num_iterations << "\n\n";

    // Create trainer
    unigram::Trainer trainer(config);
    
    // Track start time and last message
    auto start_time = std::chrono::steady_clock::now();
    std::string last_msg;
    
    // Set progress callback with progress bar
    trainer.set_progress_callback([verbose, start_time, &last_msg](size_t current, size_t total, const std::string& msg) mutable {
        // Calculate percentage
        double percentage = (total > 0) ? (100.0 * current / total) : 0.0;
        
        // Calculate elapsed time
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
        
        // Create progress bar (50 chars wide)
        const int bar_width = 50;
        int filled = static_cast<int>(bar_width * current / total);
        
        std::cout << "\r"; // Carriage return to overwrite line
        std::cout << "Progress: [";
        
        for (int i = 0; i < bar_width; ++i) {
            if (i < filled) {
                std::cout << "=";
            } else if (i == filled && current < total) {
                std::cout << ">";
            } else {
                std::cout << " ";
            }
        }
        
        std::cout << "] "
                  << std::fixed << std::setprecision(1) << percentage << "% "
                  << "(" << current << "/" << total << ") "
                  << "[" << elapsed << "s]";
        
        // Only print verbose message if it changed
        if (verbose && msg != last_msg) {
            std::cout << "\n  -> " << msg;
            last_msg = msg;
        }
        
        std::cout << std::flush;
        
        // Print newline when complete
        if (current >= total) {
            std::cout << "\n";
        }
    });

    try {
        // Train the model
        unigram::Vocabulary vocab;
        if (!input_file.empty()) {
            vocab = trainer.train_from_file(input_file);
        } else {
            vocab = trainer.train_from_directory(input_dir, pattern);
        }
        
        std::cout << "\nTraining complete!\n";
        std::cout << "Final vocabulary size: " << vocab.size() << " tokens\n";
        
        // Save the vocabulary
        if (verbose) {
            std::cout << "\nSaving model...\n";
            std::cout << "  Output file: " << output_file << "\n";
            std::cout << "  Vocabulary size: " << vocab.size() << " tokens\n";
        } else {
            std::cout << "Saving model to: " << output_file << "\n";
        }
        
        if (vocab.save(output_file)) {
            std::cout << "✓ Model saved successfully!\n";
            if (verbose) {
                // Get file size
                std::ifstream file(output_file, std::ios::binary | std::ios::ate);
                if (file.is_open()) {
                    auto size = file.tellg();
                    std::cout << "  File size: " << (size / 1024) << " KB\n";
                }
            }
            return 0;
        } else {
            std::cerr << "\n✗ Failed to save model\n";
            std::cerr << "See error details above.\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Training failed: " << e.what() << "\n";
        return 1;
    }
}
