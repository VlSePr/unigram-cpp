# Development Roadmap

This document outlines the development plan for completing the Unigram Tokeniser implementation.

## Phase 1: Core Algorithm Implementation ✅ COMPLETE

- [x] Project structure setup
- [x] CMake build system
- [x] Basic class interfaces
- [x] PIMPL pattern implementation
- [x] Unit test framework
- [x] Implement Viterbi algorithm in tokenizer
- [x] Implement EM algorithm in trainer
- [x] Multi-file training support
- [x] Vocabulary management
- [x] UTF-8 utilities (renamed from bpe_utils)

## Phase 2: Serialization & I/O ✅ COMPLETE

- [x] Add JSON library (nlohmann/json via FetchContent)
- [x] Implement Vocabulary::save()
- [x] Implement Vocabulary::load()
- [x] Add save/load tests
- [x] Integrate with CLI train command

### JSON Format
```json
{
  "version": "1.0.0",
  "vocab_size": 32000,
  "vocabulary": [
    {"id": 0, "token": "<unk>", "score": 0.0},
    {"id": 1, "token": "hello", "score": -1.5},
    {"id": 2, "token": "world", "score": -2.1}
  ]
}
```

## Phase 3: Algorithm Optimization

**Priority: Medium**
**Status: Future Enhancement**

### Viterbi Algorithm (✅ Basic implementation complete)
- [x] Implement dynamic programming for segmentation
- [ ] Add beam search option for speed
- [ ] Optimize memory usage
- [ ] Add caching for frequent patterns

### EM Training Algorithm (✅ Basic implementation complete)
- [x] E-step: Calculate expected counts using Viterbi
- [x] M-step: Update probabilities and prune vocabulary
- [x] Vocabulary pruning strategies
- [ ] Convergence criteria and early stopping
- [ ] Multi-threaded training (OpenMP)

### Performance Targets
- Tokenization: >1M tokens/second
- Training: <10 minutes for 100MB corpus
- Memory: <500MB for 32K vocabulary

## Phase 4: Advanced Features

**Priority: Medium**

### Tokenizer Features
- [ ] Byte-fallback for unknown characters
- [ ] Pre-tokenization (whitespace, punctuation)
- [ ] Special token handling
- [ ] Maximum length truncation
- [ ] Padding support

### Trainer Features
- [ ] Character coverage analysis
- [ ] Vocabulary initialization strategies
- [ ] Custom tokenization rules
- [ ] Multi-threaded training (OpenMP)

## Phase 5: Language Bindings

**Priority: Medium**

### Python Bindings
- [ ] Complete pybind11 bindings
- [ ] Add Python tests
- [ ] Create setup.py for pip installation
- [ ] Add type stubs (.pyi files)
- [ ] Documentation with Sphinx
- [ ] PyPI package

### Java Bindings
- [ ] Complete JNI implementation
- [ ] Add Java tests
- [ ] Create Maven/Gradle build
- [ ] JavaDoc documentation
- [ ] Maven Central package

## Phase 6: Testing & Quality

**Priority: High**

### Unit Tests
- [ ] Achieve >90% code coverage
- [ ] Add edge case tests
- [ ] Add Unicode test cases
- [ ] Add performance regression tests

### Integration Tests
- [ ] CLI command tests
- [ ] End-to-end training tests
- [ ] Cross-platform tests
- [ ] Binding integration tests

### Benchmarks
- [ ] Tokenization speed benchmarks
- [ ] Training speed benchmarks
- [ ] Memory usage benchmarks
- [ ] Comparison with SentencePiece/HuggingFace

## Phase 7: Documentation

**Priority: Medium**

- [ ] API documentation (Doxygen)
- [ ] Algorithm explanation
- [ ] Performance tuning guide
- [ ] Migration guide (from other tokenizers)
- [ ] Contributing guidelines
- [ ] Code of conduct

## Phase 8: CI/CD & Release

**Priority: Medium**

### Continuous Integration
- [ ] GitHub Actions workflow
- [ ] Multi-platform builds (Windows, Linux, macOS)
- [ ] Automated testing
- [ ] Code coverage reports
- [ ] Static analysis (clang-tidy, cppcheck)

### Release Process
- [ ] Versioning strategy (SemVer)
- [ ] Changelog maintenance
- [ ] Binary releases (GitHub Releases)
- [ ] Package manager support (vcpkg, conan)

## C++23 Features to Consider

Once we're ready to upgrade from C++20 to C++23:

### Error Handling
```cpp
// Replace bool returns with std::expected
std::expected<Vocabulary, Error> load(const std::string& filepath);
```

### Ranges
```cpp
// More elegant text processing
auto tokens = text 
    | std::views::split(' ')
    | std::views::transform(process_token);
```

### `std::mdspan`
```cpp
// Efficient multi-dimensional arrays for probability tables
std::mdspan<double, std::extents<size_t, dynamic, dynamic>> probs;
```

### Pattern Matching (future)
```cpp
// Cleaner token type handling
match (token_type) {
    TokenType::Word => process_word(),
    TokenType::Number => process_number(),
    _ => handle_other()
}
```

## Implementation Priorities

### Must Have (for v1.0)
1. ✅ Core Viterbi tokenization
2. ✅ EM training algorithm
3. ✅ JSON serialization
4. ✅ UTF-8 support
5. ✅ Basic tests
6. ✅ CLI tool

### Should Have (for v1.1)
1. Python bindings
2. Performance optimizations
3. Comprehensive tests
4. Documentation

### Nice to Have (for v2.0)
1. Java bindings
2. Multi-threaded training
3. Advanced features (byte-fallback, etc.)
4. Benchmarks suite

## Contributing

We welcome contributions! Please focus on:

1. **Algorithm Implementation**: Help implement core tokenization/training
2. **Testing**: Add test cases, especially edge cases
3. **Documentation**: Improve docs, add examples
4. **Bindings**: Help complete Python/Java bindings
5. **Performance**: Profile and optimize hot paths

## Timeline Estimate

- **Phase 1-2**: 2-3 weeks (Core + Serialization)
- **Phase 3**: 2-3 weeks (Optimization)
- **Phase 4**: 1-2 weeks (Advanced Features)
- **Phase 5**: 2-3 weeks (Bindings)
- **Phase 6**: 1-2 weeks (Testing)
- **Phase 7-8**: 1-2 weeks (Docs & CI/CD)

**Total**: ~10-15 weeks for complete implementation

## Notes

- Focus on correctness first, then optimize
- Keep the API stable once we reach v1.0
- Maintain backward compatibility
- Follow the existing code style
- Write tests for new features
- Document public APIs

## Questions?

Open an issue on GitHub or start a discussion!
