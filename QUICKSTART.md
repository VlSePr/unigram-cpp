# Quick Start Guide

Get started with the Unigram Tokeniser in 5 minutes!

## Step 1: Build the Project

### Windows (Visual Studio)

```cmd
# Navigate to project directory
cd UnigramTokeniser

# Create build directory and generate Visual Studio solution
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64

# Open Visual Studio
start UnigramTokeniser.sln
```

Build in Visual Studio (F7) or from command line:
```cmd
cmake --build . --config Release
```

### Linux/macOS

```bash
cd UnigramTokeniser
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

## Step 2: Run Tests

```bash
# From build directory
ctest -C Release
```

## Step 3: Try the CLI

### Train a Tokenizer

```bash
# Windows
bin\Release\unigram-cli.exe train --input ..\data\sample_corpus.txt --output model.json --vocab-size 500

# Linux/macOS
./bin/unigram-cli train --input ../data/sample_corpus.txt --output model.json --vocab-size 500
```

### Tokenize Text

```bash
# Windows
echo "Hello world" | bin\Release\unigram-cli.exe tokenize --model model.json

# Linux/macOS
echo "Hello world" | ./bin/unigram-cli tokenize --model model.json
```

### Encode to IDs

```bash
# Windows
bin\Release\unigram-cli.exe encode --model model.json --text "Hello world"

# Linux/macOS
./bin/unigram-cli encode --model model.json --text "Hello world"
```

## Step 4: Use in C++

```cpp
#include <unigram/tokenizer.h>
#include <unigram/trainer.h>
#include <iostream>

int main() {
    // Train a model
    unigram::TokenizerConfig config;
    config.vocab_size = 1000;
    
    unigram::Trainer trainer(config);
    auto vocab = trainer.train_from_file("corpus.txt");
    
    // Create tokenizer
    unigram::Tokenizer tokenizer(config);
    tokenizer.set_vocabulary(vocab);
    
    // Use it
    auto tokens = tokenizer.encode("Hello, world!");
    for (const auto& token : tokens) {
        std::cout << token << " ";
    }
    
    return 0;
}
```

**Compile your program:**

```bash
# Linux
g++ -std=c++20 -I../include -L./lib -o myapp myapp.cpp -lunigram_tokeniser

# Windows (after building the library)
cl /std:c++20 /EHsc /I..\include myapp.cpp /link unigram_tokeniser.lib
```

## Step 5: Next Steps

- **Read the [Architecture Documentation](docs/ARCHITECTURE.md)** to understand the design
- **Check [Build Instructions](docs/BUILD.md)** for advanced build options
- **Explore [Examples](examples/)** for more usage patterns
- **Build Python/Java bindings** if needed

## Common Commands Reference

### CMake Build Options

```bash
# Build with Python bindings
cmake .. -DBUILD_PYTHON_BINDINGS=ON

# Build with Java bindings
cmake .. -DBUILD_JAVA_BINDINGS=ON

# Build without tests
cmake .. -DBUILD_TESTS=OFF

# Build static library instead of DLL
cmake .. -DBUILD_SHARED_LIBS=OFF

# Debug build
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

### CLI Commands

```bash
# Show help
unigram-cli help

# Train with custom parameters
unigram-cli train --input corpus.txt --output model.json \
    --vocab-size 32000 --iterations 4 --min-frequency 2

# Tokenize from file
unigram-cli tokenize --model model.json --input input.txt --output output.txt

# Encode text
unigram-cli encode --model model.json --text "Your text here"
```

## Troubleshooting

### Build fails with "CMake version too old"
```bash
pip install --upgrade cmake
```

### "Cannot find DLL" on Windows
Add the build output directory to PATH:
```cmd
set PATH=%PATH%;C:\path\to\UnigramTokeniser\build\bin\Release
```

### Tests fail
Make sure you built in Release mode and run from the build directory:
```bash
cd build
ctest -C Release --verbose
```

## Project Structure Overview

```
UnigramTokeniser/
├── include/unigram/     # Public API headers
│   ├── tokenizer.h      # Main tokenizer class
│   ├── vocabulary.h     # Vocabulary management
│   ├── trainer.h        # Training functionality
│   └── types.h          # Common types
│
├── src/                 # Library implementation
├── cli/                 # Command-line tool
├── tests/               # Unit tests
├── bindings/            # Python & Java bindings
│   ├── python/
│   └── java/
├── examples/            # Usage examples
├── data/                # Sample data
└── docs/                # Documentation
```

## What's Next?

1. **Implement the core algorithm**: The current code has placeholder implementations
2. **Add JSON serialization**: For saving/loading models
3. **Optimize performance**: Profile and optimize hot paths
4. **Add benchmarks**: Compare with other tokenizers
5. **Complete language bindings**: Fully test Python/Java integration

Happy tokenizing! 🚀
