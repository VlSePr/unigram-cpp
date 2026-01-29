#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <vector>
#include <string>
#include <unigram/tokenizer.h>
#include <unigram/trainer.h>

namespace fs = std::filesystem;

namespace {
    struct BenchmarkResult {
        std::string name;
        double duration_ms;
        size_t operations;
        double ops_per_second;
        std::string unit;
    };

    void print_result(const BenchmarkResult& result) {
        std::cout << std::left << std::setw(40) << result.name << ": "
                  << std::right << std::setw(10) << std::fixed << std::setprecision(2) 
                  << result.duration_ms << " ms";
        
        if (result.operations > 0) {
            std::cout << "  (" << std::setw(12) << std::fixed << std::setprecision(0)
                      << result.ops_per_second << " " << result.unit << "/sec)";
        }
        std::cout << "\n";
    }

    std::vector<std::string> load_corpus(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filepath);
        }

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (!line.empty()) {
                lines.push_back(line);
            }
        }
        return lines;
    }

    size_t count_characters(const std::vector<std::string>& corpus) {
        size_t total = 0;
        for (const auto& line : corpus) {
            total += line.size();
        }
        return total;
    }
}

int benchmark_command(int argc, char* argv[]) {
    std::cout << "=================================================\n";
    std::cout << "         Unigram Tokenizer Benchmark Suite       \n";
    std::cout << "=================================================\n\n";

    // Parse arguments
    std::string corpus_file = "data/alice-in-wonderland";
    std::string model_file;
    size_t vocab_size = 2000;
    size_t warmup_runs = 3;
    size_t benchmark_runs = 10;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "--corpus" || arg == "-c") && i + 1 < argc) {
            corpus_file = argv[++i];
        } else if ((arg == "--model" || arg == "-m") && i + 1 < argc) {
            model_file = argv[++i];
        } else if (arg == "--vocab-size" && i + 1 < argc) {
            vocab_size = std::stoul(argv[++i]);
        } else if (arg == "--runs" && i + 1 < argc) {
            benchmark_runs = std::stoul(argv[++i]);
        }
    }

    std::vector<BenchmarkResult> results;

    try {
        // Load corpus
        std::cout << "Loading corpus: " << corpus_file << "\n";
        auto corpus = load_corpus(corpus_file);
        size_t total_chars = count_characters(corpus);
        std::cout << "  Lines: " << corpus.size() << "\n";
        std::cout << "  Characters: " << total_chars << "\n\n";

        // Benchmark 1: Training
        std::cout << "Benchmarking training...\n";
        {
            unigram::TokenizerConfig config;
            config.vocab_size = vocab_size;
            config.num_iterations = 4;

            unigram::Trainer trainer(config);

            auto start = std::chrono::high_resolution_clock::now();
            auto vocab = trainer.train(corpus);
            auto end = std::chrono::high_resolution_clock::now();

            double duration_ms = std::chrono::duration<double, std::milli>(end - start).count();
            double chars_per_sec = (total_chars * 1000.0) / duration_ms;

            results.push_back({
                "Training (" + std::to_string(vocab_size) + " vocab)",
                duration_ms,
                total_chars,
                chars_per_sec,
                "chars"
            });

            // Save model for tokenization benchmark if not provided
            if (model_file.empty()) {
                model_file = "model/benchmark_temp.json";
                vocab.save(model_file);
                std::cout << "  Saved temporary model: " << model_file << "\n";
            }
        }

        // Benchmark 2: Model Load
        std::cout << "Benchmarking model loading...\n";
        unigram::Vocabulary vocab;
        {
            auto start = std::chrono::high_resolution_clock::now();
            vocab.load(model_file);
            auto end = std::chrono::high_resolution_clock::now();

            double duration_ms = std::chrono::duration<double, std::milli>(end - start).count();

            results.push_back({
                "Model loading",
                duration_ms,
                0,
                0,
                ""
            });
        }

        // Benchmark 3: Tokenization
        std::cout << "Benchmarking tokenization...\n";
        {
            unigram::TokenizerConfig config;
            unigram::Tokenizer tokenizer(config);
            tokenizer.set_vocabulary(vocab);

            // Warmup
            for (size_t i = 0; i < warmup_runs; ++i) {
                for (const auto& line : corpus) {
                    tokenizer.encode(line);
                }
            }

            // Actual benchmark
            auto start = std::chrono::high_resolution_clock::now();
            for (size_t i = 0; i < benchmark_runs; ++i) {
                for (const auto& line : corpus) {
                    tokenizer.encode(line);
                }
            }
            auto end = std::chrono::high_resolution_clock::now();

            double duration_ms = std::chrono::duration<double, std::milli>(end - start).count();
            double avg_duration = duration_ms / benchmark_runs;
            double chars_per_sec = (total_chars * 1000.0) / avg_duration;

            results.push_back({
                "Tokenization (avg of " + std::to_string(benchmark_runs) + " runs)",
                avg_duration,
                total_chars,
                chars_per_sec,
                "chars"
            });
        }

        // Benchmark 4: Encoding to IDs
        std::cout << "Benchmarking encoding to IDs...\n";
        {
            unigram::TokenizerConfig config;
            unigram::Tokenizer tokenizer(config);
            tokenizer.set_vocabulary(vocab);

            // Warmup
            for (size_t i = 0; i < warmup_runs; ++i) {
                for (const auto& line : corpus) {
                    tokenizer.encode_as_ids(line);
                }
            }

            // Actual benchmark
            auto start = std::chrono::high_resolution_clock::now();
            for (size_t i = 0; i < benchmark_runs; ++i) {
                for (const auto& line : corpus) {
                    tokenizer.encode_as_ids(line);
                }
            }
            auto end = std::chrono::high_resolution_clock::now();

            double duration_ms = std::chrono::duration<double, std::milli>(end - start).count();
            double avg_duration = duration_ms / benchmark_runs;
            double chars_per_sec = (total_chars * 1000.0) / avg_duration;

            results.push_back({
                "Encode to IDs (avg of " + std::to_string(benchmark_runs) + " runs)",
                avg_duration,
                total_chars,
                chars_per_sec,
                "chars"
            });
        }

        // Print results
        std::cout << "\n=================================================\n";
        std::cout << "                  Benchmark Results               \n";
        std::cout << "=================================================\n\n";

        for (const auto& result : results) {
            print_result(result);
        }

        std::cout << "\n=================================================\n";
        std::cout << "Benchmark suite completed successfully!\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
