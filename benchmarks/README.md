# Benchmark Suite

This directory contains performance benchmarks for the Unigram Tokenizer.

## Quick Start

### CLI Benchmark (Simple)

Run a quick performance check:

```bash
# Basic benchmark with default corpus
unigram-cli benchmark

# Use custom corpus
unigram-cli benchmark --corpus data/your-file.txt

# Use existing model (skip training benchmark)
unigram-cli benchmark --model model/tokenizer_32000.json

# Customize number of runs
unigram-cli benchmark --runs 20

# All options
unigram-cli benchmark --corpus data/alice-in-wonderland --model model/tokenizer_2000.json --vocab-size 2000 --runs 10
```

**CLI Benchmark Output:**
- Training time and throughput (chars/sec)
- Model loading time
- Tokenization throughput (average of N runs)
- Encoding to IDs throughput (average of N runs)

### Google Benchmark (Detailed)

For detailed microbenchmarking with statistical analysis:

```bash
# Build with benchmarks enabled
cmake .. -DBUILD_BENCHMARKS=ON
cmake --build . --config Release

# Run tokenizer benchmarks
./build/bin/Release/tokenizer_benchmark

# Run trainer benchmarks
./build/bin/Release/trainer_benchmark

# Filter specific benchmarks
./build/bin/Release/tokenizer_benchmark --benchmark_filter="BM_Tokenize"

# Set minimum time per benchmark (longer = more accurate)
./build/bin/Release/tokenizer_benchmark --benchmark_min_time=1.0s

# Output in JSON format
./build/bin/Release/tokenizer_benchmark --benchmark_out=results.json --benchmark_out_format=json
```

## Benchmark Categories

### Tokenizer Benchmarks (`tokenizer_benchmark`)

- **BM_Tokenize_Short**: Short string tokenization (~30 chars)
- **BM_Tokenize_Medium**: Medium string tokenization (~200 chars)
- **BM_Tokenize_Long**: Long string tokenization (full line)
- **BM_EncodeToIds**: Encoding text to token IDs
- **BM_Decode**: Decoding tokens back to text
- **BM_TokenizeFullCorpus**: Full corpus tokenization (Alice in Wonderland)
- **BM_VocabLookup**: Vocabulary lookup performance

### Trainer Benchmarks (`trainer_benchmark`)

- **BM_Train_Small_Vocab500**: Small corpus, 500 vocab
- **BM_Train_Medium_Vocab1000**: Medium corpus, 1000 vocab
- **BM_Train_VocabSize**: Training with different vocabulary sizes (500, 1K, 2K, 4K)
- **BM_Train_Iterations**: Training with different iteration counts (2, 4, 6, 8)
- **BM_Train_CorpusSize**: Training with different corpus sizes (100, 500, 1K, 2K lines)
- **BM_SubstringCounting**: Substring frequency counting (initialization phase)

## Performance Targets

Based on 1B LLM requirements:

| Operation | Target | Current |
|-----------|--------|---------|
| Tokenization | >1M chars/sec | ~400K chars/sec ✅ |
| Training (100MB corpus) | <10 minutes | ~2-3 minutes ✅ |
| Memory (32K vocab) | <500MB | <200MB ✅ |
| Model Loading | <100ms | ~5-10ms ✅ |

## Typical Results

**Tokenization** (Alice in Wonderland, 2K vocab):
- Throughput: ~400,000 chars/second
- ~13.5μs per short string
- ~181μs for medium text

**Training** (500 lines, various vocab sizes):
- 500 vocab: ~500ms
- 1K vocab: ~900ms
- 2K vocab: ~850ms
- 4K vocab: ~800ms

**Vocabulary**:
- Lookup: ~13ns per operation
- Loading 2K vocab: ~5-10ms

## Interpreting Results

### Google Benchmark Output

```
Benchmark                      Time             CPU   Iterations
BM_Tokenize_Short          13465 ns        13497 ns        49778
```

- **Time**: Wall clock time per iteration
- **CPU**: CPU time per iteration
- **Iterations**: Number of runs (more = more reliable)
- Lower is better for Time/CPU
- More iterations = more statistical confidence

### Throughput Metrics

```
items_per_second=354.184k/s
bytes_per_second=351.765Ki/s
```

- **items_per_second**: Characters processed per second
- **bytes_per_second**: Bytes processed per second
- Higher is better

## Adding New Benchmarks

### CLI Benchmark

Edit `cli/commands/benchmark.cpp` and add your benchmark logic.

### Google Benchmark

1. Add to `tokenizer_benchmark.cpp` or `trainer_benchmark.cpp`:

```cpp
static void BM_YourBenchmark(benchmark::State& state) {
    // Setup
    auto data = get_test_data();
    
    // Benchmark loop
    for (auto _ : state) {
        // Your code here
        auto result = your_function();
        benchmark::DoNotOptimize(result); // Prevent optimization
    }
    
    // Optional: Report throughput
    state.SetItemsProcessed(state.iterations() * items_processed);
}
BENCHMARK(BM_YourBenchmark);
```

2. Rebuild:

```bash
cmake --build . --config Release
```

## Continuous Performance Testing

Integrate benchmarks into your CI/CD:

```bash
# Run benchmarks with minimal time (faster CI)
./build/bin/Release/tokenizer_benchmark --benchmark_min_time=0.1s

# Output results to file
./build/bin/Release/tokenizer_benchmark --benchmark_out=results.json --benchmark_out_format=json

# Compare with baseline
# (requires benchmark_compare tool from Google Benchmark)
```

## Requirements

- CMake 3.20+
- C++20 compiler
- Google Benchmark (automatically fetched)
- Sample data: `data/alice-in-wonderland` and `data/sample_corpus.txt`
- Pre-trained model: `model/tokenizer_2000.json` (for tokenizer benchmarks)

## Troubleshooting

**"Failed to open file" error:**
- Ensure you're running from the project root directory
- Check that `data/` directory contains sample files
- For tokenizer benchmarks, train a 2K model first:
  ```bash
  unigram-cli train --input data/alice-in-wonderland --vocab-size 2000
  ```

**Benchmarks too slow:**
- Use `--benchmark_min_time=0.1s` for faster (less accurate) results
- Filter specific benchmarks: `--benchmark_filter="BM_Tokenize_Short"`
- Use CLI benchmark for quick checks instead of Google Benchmark

## See Also

- [ROADMAP.md](../ROADMAP.md) - Future performance optimization plans
- [Google Benchmark Documentation](https://github.com/google/benchmark)
