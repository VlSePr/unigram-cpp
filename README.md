# Unigram Tokeniser

A high-performance C++ library implementing the Unigram tokenization algorithm with the EM training algorithm, featuring bindings for Python and Java.

## Features

- **Complete Unigram implementation** with Viterbi decoding and EM training
- **Fast C++20 implementation** using dynamic programming
- **Multi-file training** - train from directory or multiple files
- **Cross-platform** support (Windows, Linux, macOS)
- **Language bindings** for Python and Java
- **CLI tool** for training and tokenization
- **Comprehensive test suite** with Google Test

## Algorithm Overview

### Tokenization (Viterbi Algorithm)
- Uses dynamic programming to find optimal token segmentation
- Maximizes combined log probability of token sequence
- Handles unknown tokens gracefully with fallback

### Training (EM Algorithm)
1. **Initialization**: Seeds vocabulary with most frequent substrings
2. **E-step**: Calculates expected token counts using Viterbi
3. **M-step**: Updates probabilities and prunes to target vocabulary size
4. Iterates to convergence

## Building

### Prerequisites

- CMake 3.20 or higher
- C++20 compatible compiler (MSVC 2019+, GCC 10+, Clang 10+)
- (Optional) Python 3.8+ for Python bindings
- (Optional) JDK 11+ for Java bindings

### Build with CMake

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release

# Run tests
ctest -C Release
```

### Build with Visual Studio

```bash
# Generate Visual Studio solution
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"

# Open UnigramTokeniser.sln in Visual Studio
```

## Usage

### C++ Library

```cpp
#include <unigram/tokenizer.h>
#include <unigram/trainer.h>

// Training
unigram::TokenizerConfig config;
config.vocab_size = 32000;
config.num_iterations = 4;

unigram::Trainer trainer(config);
auto vocab = trainer.train_from_file("corpus.txt");

// Tokenization
unigram::Tokenizer tokenizer(config);
tokenizer.set_vocabulary(vocab);

auto tokens = tokenizer.encode("Hello, world!");
auto ids = tokenizer.encode_as_ids("Hello, world!");
auto text = tokenizer.decode(tokens);
```

### CLI Tool

```bash
# Train from a single file with custom output
unigram-cli train -i corpus.txt -o model.json --vocab-size 32000

# Train with default output (saves to model/tokenizer_<vocabsize>.json)
unigram-cli train --input-dir data --vocab-size 32000

# Train from a directory (all files)
unigram-cli train --input-dir data -o my_model.json

# Train from directory with specific pattern
unigram-cli train --input-dir data --pattern *.txt -o model.json

# Tokenize text
unigram-cli tokenize --model model.json --input text.txt

# Encode to IDs
unigram-cli encode --model model.json --text "Hello, world!"

# Benchmark performance
unigram-cli benchmark

# Quiet mode - just the progress bar
unigram-cli train --input-dir data

# Verbose mode - see all the details!
unigram-cli train --input-dir data --verbose

# Or use short form
unigram-cli train --input-dir data -v
```

**Default Output Directory:**
- Models are automatically saved to `model/` directory
- Default filename format: `tokenizer_<vocabsize>.json`
- Example: `--vocab-size 32000` → saves to `model/tokenizer_32000.json`
- Override with `--output` or `-o` flag

## Performance

Benchmarked on Windows 11, Intel Core i7 (8 cores, 2.99GHz):

| Operation | Performance |
|-----------|-------------|
| **Tokenization** | ~400,000 chars/sec |
| **Encoding to IDs** | ~350,000 chars/sec |
| **Training** (2K vocab, 166K chars) | ~2-3 seconds |
| **Model Loading** (2K vocab) | ~5-10 ms |
| **Memory Usage** (2K vocab) | <50 MB |

**Run benchmarks:**

```bash
# Quick CLI benchmark
unigram-cli benchmark

# Detailed Google Benchmark suite
cmake .. -DBUILD_BENCHMARKS=ON
cmake --build . --config Release
./build/bin/Release/tokenizer_benchmark
./build/bin/Release/trainer_benchmark
```

See [benchmarks/README.md](benchmarks/README.md) for detailed benchmarking guide.

### Python Bindings (Coming Soon)

```python
import unigram

tokenizer = unigram.Tokenizer.load("model.json")
tokens = tokenizer.encode("Hello, world!")
```

### Java Bindings (Coming Soon)

```java
import com.unigram.Tokenizer;

Tokenizer tokenizer = Tokenizer.load("model.json");
List<String> tokens = tokenizer.encode("Hello, world!");
```

## Project Structure

```
UnigramTokeniser/
├── include/unigram/    # Public API headers
│   ├── tokenizer.h     # Viterbi tokenization
│   ├── trainer.h       # EM training algorithm
│   ├── vocabulary.h    # Token management
│   └── types.h         # Common types
├── src/                # Library implementation
│   ├── tokenizer.cpp   # ✅ Viterbi algorithm implemented
│   ├── trainer.cpp     # ✅ EM algorithm implemented
│   ├── vocabulary.cpp  # ✅ Complete
│   └── internal/       # UTF-8 utilities
├── cli/                # Command-line tool
│   ├── main.cpp        # Command dispatcher
│   └── commands/       # train, tokenize, encode, benchmark
├── tests/              # Unit tests (Google Test)
├── benchmarks/         # Performance benchmarks (Google Benchmark + CLI)
├── bindings/           # Language bindings (Python, Java)
├── examples/           # Usage examples
├── data/               # Training corpus (14 classic texts)
├── model/              # Default output directory for trained models
└── docs/               # Documentation
```

## Current Status

✅ **Complete:**
- Viterbi tokenization algorithm
- EM training algorithm (E-step, M-step, pruning)
- Multi-file training support
- Vocabulary management
- **JSON serialization (save/load models)**
- **UTF-8 character-aware processing**
- **Word-focused tokenization (prioritizes complete words)**
- CLI tool (train, tokenize, encode, benchmark)
- **Benchmark suite** (CLI + Google Benchmark)
- Test suite (27 tests passing)

🚧 **In Progress:**
- Python bindings (scaffolded)
- Java bindings (scaffolded)

## Next Steps

See [ROADMAP.md](ROADMAP.md) for detailed development plan.

## License

[Specify your license here]

## Contributing

Contributions are welcome! Please read the contributing guidelines first.
