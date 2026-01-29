#include <benchmark/benchmark.h>
#include <unigram/tokenizer.h>
#include <unigram/vocabulary.h>
#include <fstream>
#include <vector>
#include <string>

namespace {
    // Load test corpus
    std::vector<std::string> load_test_corpus(const std::string& path) {
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
        }
        
        return lines;
    }

    // Shared test data
    struct TestData {
        unigram::Vocabulary vocab;
        std::vector<std::string> corpus;
        
        TestData() {
            // Load pre-trained model
            vocab.load("model/tokenizer_2000.json");
            
            // Load test corpus
            corpus = load_test_corpus("data/alice-in-wonderland");
        }
    };

    TestData& get_test_data() {
        static TestData data;
        return data;
    }
}

// Benchmark: Tokenize single short string
static void BM_Tokenize_Short(benchmark::State& state) {
    auto& data = get_test_data();
    unigram::TokenizerConfig config;
    unigram::Tokenizer tokenizer(config);
    tokenizer.set_vocabulary(data.vocab);
    
    const std::string text = "Hello, world! This is a test.";
    
    for (auto _ : state) {
        auto tokens = tokenizer.encode(text);
        benchmark::DoNotOptimize(tokens);
    }
    
    state.SetItemsProcessed(state.iterations() * text.size());
}
BENCHMARK(BM_Tokenize_Short);

// Benchmark: Tokenize medium string
static void BM_Tokenize_Medium(benchmark::State& state) {
    auto& data = get_test_data();
    unigram::TokenizerConfig config;
    unigram::Tokenizer tokenizer(config);
    tokenizer.set_vocabulary(data.vocab);
    
    const std::string text = "Alice was beginning to get very tired of sitting by her sister on the "
                           "bank, and of having nothing to do: once or twice she had peeped into the "
                           "book her sister was reading, but it had no pictures or conversations in it.";
    
    for (auto _ : state) {
        auto tokens = tokenizer.encode(text);
        benchmark::DoNotOptimize(tokens);
    }
    
    state.SetItemsProcessed(state.iterations() * text.size());
}
BENCHMARK(BM_Tokenize_Medium);

// Benchmark: Tokenize long string
static void BM_Tokenize_Long(benchmark::State& state) {
    auto& data = get_test_data();
    unigram::TokenizerConfig config;
    unigram::Tokenizer tokenizer(config);
    tokenizer.set_vocabulary(data.vocab);
    
    // Use first line from corpus (typically longer)
    const std::string& text = data.corpus[0];
    
    for (auto _ : state) {
        auto tokens = tokenizer.encode(text);
        benchmark::DoNotOptimize(tokens);
    }
    
    state.SetItemsProcessed(state.iterations() * text.size());
}
BENCHMARK(BM_Tokenize_Long);

// Benchmark: Encode to IDs
static void BM_EncodeToIds(benchmark::State& state) {
    auto& data = get_test_data();
    unigram::TokenizerConfig config;
    unigram::Tokenizer tokenizer(config);
    tokenizer.set_vocabulary(data.vocab);
    
    const std::string text = "Alice was beginning to get very tired of sitting by her sister.";
    
    for (auto _ : state) {
        auto ids = tokenizer.encode_as_ids(text);
        benchmark::DoNotOptimize(ids);
    }
    
    state.SetItemsProcessed(state.iterations() * text.size());
}
BENCHMARK(BM_EncodeToIds);

// Benchmark: Decode from tokens
static void BM_Decode(benchmark::State& state) {
    auto& data = get_test_data();
    unigram::TokenizerConfig config;
    unigram::Tokenizer tokenizer(config);
    tokenizer.set_vocabulary(data.vocab);
    
    const std::string original = "Alice was beginning to get very tired of sitting by her sister.";
    auto tokens = tokenizer.encode(original);
    
    for (auto _ : state) {
        auto text = tokenizer.decode(tokens);
        benchmark::DoNotOptimize(text);
    }
    
    state.SetItemsProcessed(state.iterations() * original.size());
}
BENCHMARK(BM_Decode);

// Benchmark: Full corpus tokenization
static void BM_TokenizeFullCorpus(benchmark::State& state) {
    auto& data = get_test_data();
    unigram::TokenizerConfig config;
    unigram::Tokenizer tokenizer(config);
    tokenizer.set_vocabulary(data.vocab);
    
    size_t total_chars = 0;
    for (const auto& line : data.corpus) {
        total_chars += line.size();
    }
    
    for (auto _ : state) {
        for (const auto& line : data.corpus) {
            auto tokens = tokenizer.encode(line);
            benchmark::DoNotOptimize(tokens);
        }
    }
    
    state.SetItemsProcessed(state.iterations() * total_chars);
    state.SetBytesProcessed(state.iterations() * total_chars);
}
BENCHMARK(BM_TokenizeFullCorpus);

// Benchmark: Vocabulary lookup performance with different sizes
static void BM_VocabLookup(benchmark::State& state) {
    auto& data = get_test_data();
    
    const std::string token = "the";
    
    for (auto _ : state) {
        auto id = data.vocab.get_id(token);
        benchmark::DoNotOptimize(id);
    }
}
BENCHMARK(BM_VocabLookup);

BENCHMARK_MAIN();
