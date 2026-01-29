#include <benchmark/benchmark.h>
#include <unigram/trainer.h>
#include <fstream>
#include <vector>
#include <string>

namespace {
    std::vector<std::string> load_corpus(const std::string& path, size_t max_lines = 0) {
        std::ifstream file(path, std::ios::binary);
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
}

// Benchmark: Training with small corpus and vocabulary
static void BM_Train_Small_Vocab500(benchmark::State& state) {
    auto corpus = load_corpus("data/sample_corpus.txt");
    
    for (auto _ : state) {
        unigram::TokenizerConfig config;
        config.vocab_size = 500;
        config.num_iterations = 4;
        
        unigram::Trainer trainer(config);
        auto vocab = trainer.train(corpus);
        benchmark::DoNotOptimize(vocab);
    }
}
BENCHMARK(BM_Train_Small_Vocab500);

// Benchmark: Training with medium corpus
static void BM_Train_Medium_Vocab1000(benchmark::State& state) {
    auto corpus = load_corpus("data/alice-in-wonderland", 1000);
    
    for (auto _ : state) {
        unigram::TokenizerConfig config;
        config.vocab_size = 1000;
        config.num_iterations = 4;
        
        unigram::Trainer trainer(config);
        auto vocab = trainer.train(corpus);
        benchmark::DoNotOptimize(vocab);
    }
}
BENCHMARK(BM_Train_Medium_Vocab1000)->Unit(benchmark::kMillisecond);

// Benchmark: Training with different vocabulary sizes
static void BM_Train_VocabSize(benchmark::State& state) {
    auto corpus = load_corpus("data/alice-in-wonderland", 500);
    size_t vocab_size = state.range(0);
    
    for (auto _ : state) {
        unigram::TokenizerConfig config;
        config.vocab_size = vocab_size;
        config.num_iterations = 4;
        
        unigram::Trainer trainer(config);
        auto vocab = trainer.train(corpus);
        benchmark::DoNotOptimize(vocab);
    }
}
BENCHMARK(BM_Train_VocabSize)
    ->Args({500})
    ->Args({1000})
    ->Args({2000})
    ->Args({4000})
    ->Unit(benchmark::kMillisecond);

// Benchmark: Training with different iteration counts
static void BM_Train_Iterations(benchmark::State& state) {
    auto corpus = load_corpus("data/alice-in-wonderland", 500);
    size_t num_iterations = state.range(0);
    
    for (auto _ : state) {
        unigram::TokenizerConfig config;
        config.vocab_size = 1000;
        config.num_iterations = num_iterations;
        
        unigram::Trainer trainer(config);
        auto vocab = trainer.train(corpus);
        benchmark::DoNotOptimize(vocab);
    }
}
BENCHMARK(BM_Train_Iterations)
    ->Args({2})
    ->Args({4})
    ->Args({6})
    ->Args({8})
    ->Unit(benchmark::kMillisecond);

// Benchmark: Training with different corpus sizes
static void BM_Train_CorpusSize(benchmark::State& state) {
    size_t corpus_size = state.range(0);
    auto corpus = load_corpus("data/alice-in-wonderland", corpus_size);
    
    for (auto _ : state) {
        unigram::TokenizerConfig config;
        config.vocab_size = 1000;
        config.num_iterations = 4;
        
        unigram::Trainer trainer(config);
        auto vocab = trainer.train(corpus);
        benchmark::DoNotOptimize(vocab);
    }
    
    size_t total_chars = 0;
    for (const auto& line : corpus) {
        total_chars += line.size();
    }
    state.SetBytesProcessed(state.iterations() * total_chars);
}
BENCHMARK(BM_Train_CorpusSize)
    ->Args({100})
    ->Args({500})
    ->Args({1000})
    ->Args({2000})
    ->Unit(benchmark::kMillisecond);

// Benchmark: Substring frequency counting (initialization phase)
static void BM_SubstringCounting(benchmark::State& state) {
    auto corpus = load_corpus("data/alice-in-wonderland", 500);
    
    for (auto _ : state) {
        unigram::TokenizerConfig config;
        config.vocab_size = 2000;
        config.num_iterations = 1;  // Only one iteration to measure initialization
        
        unigram::Trainer trainer(config);
        auto vocab = trainer.train(corpus);
        benchmark::DoNotOptimize(vocab);
    }
}
BENCHMARK(BM_SubstringCounting)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
