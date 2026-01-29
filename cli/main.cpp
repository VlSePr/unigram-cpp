#include "commands/commands.h"
#include "utils/args_parser.h"
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

// Enable memory leak detection in Debug mode (Windows only)
#if defined(_WIN32) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

void print_usage() {
    std::cout << "Unigram Tokeniser CLI\n\n";
    std::cout << "Usage: unigram-cli <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  train      Train a new tokenizer model\n";
    std::cout << "  tokenize   Tokenize input text\n";
    std::cout << "  encode     Encode text to token IDs\n";
    std::cout << "  decode     Decode token IDs to text\n";
    std::cout << "  compare    Compare quality of multiple models\n";
    std::cout << "  benchmark  Run performance benchmarks\n";
    std::cout << "  help       Show this help message\n\n";
    std::cout << "Use 'unigram-cli <command> --help' for more information on a command.\n";
}

int main(int argc, char* argv[]) {
    // Enable memory leak detection in Debug builds
    #if defined(_WIN32) && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    // Optional: Set breakpoint on specific allocation number
    // _CrtSetBreakAlloc(123);
    #endif
    
    // Enable UTF-8 output on Windows
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    #endif
    
    try {
        if (argc < 2) {
            print_usage();
            return 1;
        }

        std::string command = argv[1];

        if (command == "train") {
            return train_command(argc - 1, argv + 1);
        } else if (command == "tokenize") {
            return tokenize_command(argc - 1, argv + 1);
        } else if (command == "encode") {
            return encode_command(argc - 1, argv + 1);
        } else if (command == "decode") {
            std::cerr << "Decode command not yet implemented\n";
            return 1;
        } else if (command == "compare") {
            return compare_command(argc - 1, argv + 1);
        } else if (command == "benchmark" || command == "bench") {
            return benchmark_command(argc - 1, argv + 1);
        } else if (command == "help" || command == "--help" || command == "-h") {
            print_usage();
            return 0;
        } else {
            std::cerr << "Unknown command: " << command << "\n\n";
            print_usage();
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
