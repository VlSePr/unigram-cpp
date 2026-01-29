#include "commands.h"
#include "../utils/args_parser.h"
#include <unigram/tokenizer.h>
#include <unigram/vocabulary.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <unordered_set>
#include <algorithm>
#include <filesystem>

namespace {
    struct ModelMetrics {
        std::string model_name;
        size_t vocab_size;
        size_t total_tokens;
        size_t total_chars;
        size_t unknown_count;
        size_t unique_tokens_used;
        double compression_ratio;      // chars per token
        double unknown_rate;           // percentage of <unk> tokens
        double vocab_utilization;      // percentage of vocab actually used
        double avg_token_length;       // average characters per token
    };

    std::vector<std::string> load_test_corpus(const std::string& path, size_t max_lines = 0) {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open corpus: " + path);
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
            
            if (max_lines > 0 && lines.size() >= max_lines) {
                break;
            }
        }
        
        return lines;
    }

    ModelMetrics evaluate_model(const std::string& model_path, const std::vector<std::string>& corpus) {
        ModelMetrics metrics;
        metrics.model_name = std::filesystem::path(model_path).filename().string();
        
        // Load model
        unigram::Vocabulary vocab;
        if (!vocab.load(model_path)) {
            throw std::runtime_error("Failed to load model: " + model_path);
        }
        
        metrics.vocab_size = vocab.size();
        
        // Create tokenizer
        unigram::TokenizerConfig config;
        unigram::Tokenizer tokenizer(config);
        tokenizer.set_vocabulary(vocab);
        
        // Evaluate on corpus
        metrics.total_tokens = 0;
        metrics.total_chars = 0;
        metrics.unknown_count = 0;
        std::unordered_set<std::string> used_tokens;
        
        for (const auto& line : corpus) {
            metrics.total_chars += line.size();
            auto tokens = tokenizer.encode(line);
            metrics.total_tokens += tokens.size();
            
            for (const auto& token : tokens) {
                used_tokens.insert(token);
                if (token == "<unk>") {
                    metrics.unknown_count++;
                }
            }
        }
        
        metrics.unique_tokens_used = used_tokens.size();
        metrics.compression_ratio = static_cast<double>(metrics.total_chars) / metrics.total_tokens;
        metrics.unknown_rate = (static_cast<double>(metrics.unknown_count) / metrics.total_tokens) * 100.0;
        metrics.vocab_utilization = (static_cast<double>(metrics.unique_tokens_used) / metrics.vocab_size) * 100.0;
        metrics.avg_token_length = static_cast<double>(metrics.total_chars) / metrics.total_tokens;
        
        return metrics;
    }

    void print_comparison_table(const std::vector<ModelMetrics>& all_metrics) {
        std::cout << "\n";
        std::cout << "========================================================================================================\n";
        std::cout << "                                    TOKENIZER QUALITY COMPARISON\n";
        std::cout << "========================================================================================================\n\n";
        
        // Header
        std::cout << std::left << std::setw(30) << "Model"
                  << std::right << std::setw(10) << "Vocab"
                  << std::setw(12) << "Tokens"
                  << std::setw(14) << "Compression"
                  << std::setw(12) << "Unknown%"
                  << std::setw(14) << "Vocab Use%"
                  << "\n";
        std::cout << std::string(92, '-') << "\n";
        
        // Best metrics tracking
        double best_compression = 0;
        double best_unknown = 100;
        double best_vocab_use = 0;
        
        for (const auto& m : all_metrics) {
            best_compression = std::max(best_compression, m.compression_ratio);
            best_unknown = std::min(best_unknown, m.unknown_rate);
            best_vocab_use = std::max(best_vocab_use, m.vocab_utilization);
        }
        
        // Print each model's metrics
        for (const auto& m : all_metrics) {
            std::cout << std::left << std::setw(30) << m.model_name
                      << std::right << std::setw(10) << m.vocab_size
                      << std::setw(12) << m.total_tokens;
            
            // Highlight best compression ratio
            std::cout << std::setw(14) << std::fixed << std::setprecision(2) << m.compression_ratio;
            if (m.compression_ratio == best_compression) std::cout << " ✓";
            else std::cout << "  ";
            
            // Highlight best unknown rate
            std::cout << std::setw(11) << std::fixed << std::setprecision(3) << m.unknown_rate;
            if (m.unknown_rate == best_unknown) std::cout << " ✓";
            else std::cout << "  ";
            
            // Highlight best vocab utilization
            std::cout << std::setw(11) << std::fixed << std::setprecision(1) << m.vocab_utilization;
            if (m.vocab_utilization == best_vocab_use) std::cout << " ✓";
            else std::cout << "  ";
            
            std::cout << "\n";
        }
        
        std::cout << "\n";
        std::cout << "Metrics Explanation:\n";
        std::cout << "  • Compression: Characters per token (higher = better compression)\n";
        std::cout << "  • Unknown%: Percentage of <unk> tokens (lower = better coverage)\n";
        std::cout << "  • Vocab Use%: Percentage of vocabulary actually used (higher = better efficiency)\n";
        std::cout << "  ✓ indicates best score in category\n";
        std::cout << "\n";
    }

    void print_detailed_metrics(const std::vector<ModelMetrics>& all_metrics) {
        std::cout << "========================================================================================================\n";
        std::cout << "                                      DETAILED METRICS\n";
        std::cout << "========================================================================================================\n\n";
        
        for (const auto& m : all_metrics) {
            std::cout << "Model: " << m.model_name << "\n";
            std::cout << std::string(50, '-') << "\n";
            std::cout << "  Vocabulary Size:        " << std::setw(10) << m.vocab_size << "\n";
            std::cout << "  Tokens Used:            " << std::setw(10) << m.unique_tokens_used 
                      << " (" << std::fixed << std::setprecision(1) << m.vocab_utilization << "%)\n";
            std::cout << "  Total Tokens Generated: " << std::setw(10) << m.total_tokens << "\n";
            std::cout << "  Total Characters:       " << std::setw(10) << m.total_chars << "\n";
            std::cout << "  Unknown Tokens:         " << std::setw(10) << m.unknown_count 
                      << " (" << std::fixed << std::setprecision(3) << m.unknown_rate << "%)\n";
            std::cout << "  Compression Ratio:      " << std::setw(10) << std::fixed << std::setprecision(2) 
                      << m.compression_ratio << " chars/token\n";
            std::cout << "  Avg Token Length:       " << std::setw(10) << std::fixed << std::setprecision(2) 
                      << m.avg_token_length << " chars\n";
            std::cout << "\n";
        }
    }

    void show_sample_tokenization(const std::string& model_path, const std::vector<std::string>& samples) {
        unigram::Vocabulary vocab;
        vocab.load(model_path);
        
        unigram::TokenizerConfig config;
        unigram::Tokenizer tokenizer(config);
        tokenizer.set_vocabulary(vocab);
        
        std::cout << "Model: " << std::filesystem::path(model_path).filename().string() << "\n";
        std::cout << std::string(50, '-') << "\n";
        
        for (const auto& text : samples) {
            auto tokens = tokenizer.encode(text);
            std::cout << "Input:  \"" << text << "\"\n";
            std::cout << "Tokens: [";
            for (size_t i = 0; i < tokens.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << "\"" << tokens[i] << "\"";
            }
            std::cout << "]\n";
            std::cout << "Count:  " << tokens.size() << " tokens\n\n";
        }
    }
}

void print_compare_usage() {
    std::cout << "Usage: unigram-cli compare [options]\n\n";
    std::cout << "Compare quality of multiple tokenizer models.\n\n";
    std::cout << "Options:\n";
    std::cout << "  --models <file1,file2,...>  Comma-separated list of model files to compare\n";
    std::cout << "  --corpus <file>             Test corpus for evaluation (required)\n";
    std::cout << "  --max-lines <num>           Maximum lines to evaluate (default: all)\n";
    std::cout << "  --samples <text1|text2>     Show sample tokenizations (| separated)\n";
    std::cout << "  --detailed                  Show detailed metrics for each model\n";
    std::cout << "  --help, -h                  Show this help message\n\n";
    std::cout << "Examples:\n";
    std::cout << "  Compare two models:\n";
    std::cout << "    unigram-cli compare --models model1.json,model2.json --corpus test.txt\n\n";
    std::cout << "  With sample outputs:\n";
    std::cout << "    unigram-cli compare --models model1.json,model2.json --corpus test.txt \\\n";
    std::cout << "      --samples \"Hello world|The quick brown fox\"\n\n";
    std::cout << "  Detailed comparison:\n";
    std::cout << "    unigram-cli compare --models v1.json,v2.json --corpus test.txt --detailed\n";
}

int compare_command(int argc, char* argv[]) {
    ArgsParser parser(argc, argv);

    if (parser.has_flag("--help") || parser.has_flag("-h")) {
        print_compare_usage();
        return 0;
    }

    std::string models_str = parser.get_value("--models", "");
    std::string corpus_path = parser.get_value("--corpus", "");
    std::string samples_str = parser.get_value("--samples", "");
    size_t max_lines = parser.get_int("--max-lines", 0);
    bool detailed = parser.has_flag("--detailed");

    if (models_str.empty()) {
        std::cerr << "Error: --models is required\n\n";
        print_compare_usage();
        return 1;
    }

    if (corpus_path.empty()) {
        std::cerr << "Error: --corpus is required\n\n";
        print_compare_usage();
        return 1;
    }

    // Parse model list
    std::vector<std::string> model_paths;
    size_t pos = 0;
    while (pos < models_str.size()) {
        size_t comma = models_str.find(',', pos);
        if (comma == std::string::npos) {
            model_paths.push_back(models_str.substr(pos));
            break;
        }
        model_paths.push_back(models_str.substr(pos, comma - pos));
        pos = comma + 1;
    }

    if (model_paths.size() < 2) {
        std::cerr << "Error: At least 2 models required for comparison\n";
        return 1;
    }

    try {
        // Load corpus
        std::cout << "Loading test corpus: " << corpus_path << "\n";
        auto corpus = load_test_corpus(corpus_path, max_lines);
        std::cout << "  Lines loaded: " << corpus.size() << "\n";
        
        size_t total_chars = 0;
        for (const auto& line : corpus) {
            total_chars += line.size();
        }
        std::cout << "  Total characters: " << total_chars << "\n";

        // Evaluate each model
        std::vector<ModelMetrics> all_metrics;
        for (const auto& model_path : model_paths) {
            std::cout << "Evaluating: " << model_path << "...\n";
            auto metrics = evaluate_model(model_path, corpus);
            all_metrics.push_back(metrics);
        }

        // Print comparison table
        print_comparison_table(all_metrics);

        // Print detailed metrics if requested
        if (detailed) {
            print_detailed_metrics(all_metrics);
        }

        // Show sample tokenizations if requested
        if (!samples_str.empty()) {
            std::cout << "========================================================================================================\n";
            std::cout << "                                    SAMPLE TOKENIZATIONS\n";
            std::cout << "========================================================================================================\n\n";
            
            // Parse samples
            std::vector<std::string> samples;
            pos = 0;
            while (pos < samples_str.size()) {
                size_t pipe = samples_str.find('|', pos);
                if (pipe == std::string::npos) {
                    samples.push_back(samples_str.substr(pos));
                    break;
                }
                samples.push_back(samples_str.substr(pos, pipe - pos));
                pos = pipe + 1;
            }

            for (const auto& model_path : model_paths) {
                show_sample_tokenization(model_path, samples);
            }
        }

        // Print recommendation
        std::cout << "========================================================================================================\n";
        std::cout << "RECOMMENDATION:\n";
        std::cout << "========================================================================================================\n";
        
        auto best_compression = std::max_element(all_metrics.begin(), all_metrics.end(),
            [](const ModelMetrics& a, const ModelMetrics& b) { return a.compression_ratio < b.compression_ratio; });
        auto best_unknown = std::min_element(all_metrics.begin(), all_metrics.end(),
            [](const ModelMetrics& a, const ModelMetrics& b) { return a.unknown_rate < b.unknown_rate; });
        
        std::cout << "\nBest Compression:  " << best_compression->model_name 
                  << " (" << std::fixed << std::setprecision(2) << best_compression->compression_ratio << " chars/token)\n";
        std::cout << "Best Coverage:     " << best_unknown->model_name 
                  << " (" << std::fixed << std::setprecision(3) << best_unknown->unknown_rate << "% unknown)\n";
        
        if (best_compression == best_unknown) {
            std::cout << "\n🏆 WINNER: " << best_compression->model_name << " (best on both metrics)\n";
        } else {
            std::cout << "\n💡 For production use:\n";
            std::cout << "   - Choose " << best_compression->model_name << " if compression is priority\n";
            std::cout << "   - Choose " << best_unknown->model_name << " if vocabulary coverage is priority\n";
        }
        std::cout << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
