# Architecture Overview

This document describes the architecture and design of the Unigram Tokeniser library.

## Core Components

### 1. Vocabulary (`vocabulary.h/cpp`)

Manages the mapping between tokens and their IDs, along with scores.

**Key Features:**
- Bidirectional mapping (token ↔ ID)
- Score storage for probability-based selection
- Special tokens (UNK, BOS, EOS, PAD)
- JSON serialization/deserialization

**Design Pattern:** PIMPL (Pointer to Implementation)
- Hides implementation details
- Provides ABI stability
- Enables forward compatibility

### 2. Tokenizer (`tokenizer.h/cpp`)

Main tokenization engine implementing the Unigram algorithm.

**Key Features:**
- Viterbi algorithm for optimal segmentation
- Configurable special token handling
- Efficient UTF-8 support
- Move semantics for performance

**Algorithm:**
The tokenizer uses dynamic programming (Viterbi) to find the most probable segmentation:
1. For each position in text, calculate best path
2. Use vocabulary scores as probabilities
3. Select path with maximum likelihood

### 3. Trainer (`trainer.h/cpp`)

Trains Unigram models using the EM algorithm.

**Training Process:**
1. **Initialization:** Character-level vocabulary
2. **E-step:** Calculate expected counts using current model
3. **M-step:** Update probabilities based on counts
4. **Pruning:** Remove low-probability tokens
5. **Repeat:** Until convergence or max iterations

**Features:**
- Progress callbacks for monitoring
- Configurable vocabulary size
- Frequency-based filtering
- Support for large corpora

### 4. Types (`types.h`)

Common type definitions and configuration.

**Key Types:**
- `TokenId`: uint32_t for token identifiers
- `Token`: std::string for token text
- `Score`: double for log probabilities
- `TokenizerConfig`: Configuration parameters

## Design Principles

### 1. PIMPL Pattern

All main classes use PIMPL to:
- Hide implementation details
- Reduce compilation dependencies
- Provide ABI stability
- Enable easier testing

```cpp
class Tokenizer {
public:
    // Public interface
private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
```

### 2. Modern C++20 Features

**Used Features:**
- `std::optional` for nullable values
- `constexpr` for compile-time constants
- Move semantics for efficiency
- Smart pointers for memory safety

**Future C++23 Opportunities:**
- `std::expected` for error handling
- `std::mdspan` for multi-dimensional arrays
- Ranges improvements for text processing

### 3. Cross-Platform Design

**Platform Abstraction:**
- CMake for build system
- Standard C++ (no platform-specific code)
- UTF-8 everywhere
- DLL export handling via CMake

### 4. Performance Considerations

**Optimizations:**
- Reserve capacity for vectors
- Pass by const reference
- Move semantics where applicable
- Efficient UTF-8 handling
- Minimal allocations in hot paths

## Module Interactions

```
┌─────────────┐
│ Application │
└──────┬──────┘
       │
       ▼
┌──────────────┐     ┌──────────────┐
│  Tokenizer   │────▶│  Vocabulary  │
└──────┬───────┘     └──────────────┘
       │
       │ uses
       ▼
┌──────────────┐
│  BPE Utils   │
└──────────────┘

┌──────────────┐     ┌──────────────┐
│   Trainer    │────▶│  Vocabulary  │
└──────────────┘     └──────────────┘
```

## Language Bindings Architecture

### Python Bindings (pybind11)

```
Python Code
    │
    ▼
┌─────────────┐
│  unigram.so │ (pybind11 module)
└──────┬──────┘
       │
       ▼
┌─────────────┐
│ libunigram  │ (C++ library)
└─────────────┘
```

**Advantages:**
- Automatic type conversion
- Pythonic API
- Exception translation
- Minimal boilerplate

### Java Bindings (JNI)

```
Java Code
    │
    ▼
┌──────────────┐
│ Tokenizer    │ (Java class)
│ .class       │
└──────┬───────┘
       │ JNI
       ▼
┌──────────────┐
│ unigram_jni  │ (JNI wrapper)
│ .dll/.so     │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│ libunigram   │ (C++ library)
└──────────────┘
```

**Challenges:**
- Manual memory management
- Exception handling
- Type conversions
- GC interaction

## Threading Model

**Current:** Single-threaded
**Future Considerations:**
- Parallel training with OpenMP
- Thread-safe vocabulary access
- Concurrent tokenization batches

## Memory Management

**Strategy:**
- RAII for all resources
- Smart pointers (unique_ptr, shared_ptr)
- No manual new/delete in public API
- Move semantics for large objects

## Error Handling

**Current Approach:**
- Return bool for I/O operations
- Exceptions for invalid arguments
- Assertions for internal errors

**Future with C++23:**
- `std::expected<T, Error>` for better error propagation
- More informative error types

## Testing Strategy

**Levels:**
1. **Unit Tests:** Individual class testing
2. **Integration Tests:** Component interaction
3. **Performance Tests:** Benchmark suite
4. **Binding Tests:** Python/Java interface

## Build System

**CMake Structure:**
```
CMakeLists.txt (root)
├── src/CMakeLists.txt (library)
├── cli/CMakeLists.txt (CLI tool)
├── tests/CMakeLists.txt (tests)
└── bindings/
    ├── python/CMakeLists.txt
    └── java/CMakeLists.txt
```

**Features:**
- Modular build configuration
- Optional components
- External dependency fetching
- Cross-platform support
