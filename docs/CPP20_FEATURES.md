# C++20 Features Used - Quick Reference

This document explains the C++20 features used in this project to help you understand and learn from the code.

## 1. `std::optional` - Nullable Values

**Location:** [types.h](include/unigram/types.h#L19)

```cpp
struct TokenizerConfig {
    std::optional<size_t> max_length = std::nullopt;
};
```

**Why:** Represents values that may or may not be present, safer than pointers or magic values.

**Usage:**
```cpp
if (config.max_length.has_value()) {
    size_t len = config.max_length.value();
    // or: size_t len = *config.max_length;
}

// Set value
config.max_length = 512;

// Clear value
config.max_length = std::nullopt;
```

**C++23 Alternative:** `std::expected<T, E>` for cases where you need error information.

## 2. `constexpr` - Compile-time Constants

**Location:** [types.h](include/unigram/types.h#L11-14)

```cpp
constexpr TokenId UNK_TOKEN_ID = 0;
constexpr TokenId BOS_TOKEN_ID = 1;
```

**Why:** Guarantees compile-time evaluation, better than macros.

**Usage:**
```cpp
// These are computed at compile time
constexpr int square(int x) { return x * x; }
constexpr int result = square(5); // Evaluated at compile time
```

## 3. `inline constexpr` - Header-only Constants

**Location:** [types.h](include/unigram/types.h#L17-20)

```cpp
inline constexpr const char* UNK_TOKEN = "<unk>";
inline constexpr const char* BOS_TOKEN = "<s>";
```

**Why:** Allows constants in headers without ODR violations. Better than `static const`.

**Pre-C++17:** Would need to define in .cpp file or use static
**C++17+:** Can use `inline constexpr` in headers

## 4. Move Semantics with `noexcept`

**Location:** [tokenizer.h](include/unigram/tokenizer.h#L25-26)

```cpp
Tokenizer(Tokenizer&&) noexcept;
Tokenizer& operator=(Tokenizer&&) noexcept;
```

**Why:** Enables efficient transfer of resources, `noexcept` allows optimizations.

**Usage:**
```cpp
Tokenizer tokenizer1;
Tokenizer tokenizer2 = std::move(tokenizer1); // Move, not copy
// tokenizer1 is now in valid but unspecified state
```

**Important:** `noexcept` guarantees no exceptions, enabling std::vector optimizations.

## 5. PIMPL with `std::unique_ptr`

**Location:** [tokenizer.h](include/unigram/tokenizer.h#L72-74), [tokenizer.cpp](src/tokenizer.cpp#L7-8)

```cpp
// Header
class Tokenizer {
private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

// Implementation
struct Tokenizer::Impl {
    TokenizerConfig config;
    Vocabulary vocab;
};
```

**Why:**
- Hides implementation details
- Reduces compilation dependencies
- Provides ABI stability
- Automatic memory management

## 6. `std::function` - Function Objects

**Location:** [trainer.h](include/unigram/trainer.h#L11-12)

```cpp
using ProgressCallback = std::function<void(size_t, size_t, const std::string&)>;
```

**Why:** Type-erased callable wrapper, can store lambdas, function pointers, functors.

**Usage:**
```cpp
trainer.set_progress_callback([](size_t current, size_t total, const std::string& msg) {
    std::cout << current << "/" << total << ": " << msg << "\n";
});
```

## 7. Structured Bindings (Not used yet, but useful!)

**Not yet used, but recommended for C++17+**

```cpp
// Example for future use
std::unordered_map<Token, TokenId> map;
for (const auto& [token, id] : map) {
    std::cout << token << " -> " << id << "\n";
}
```

## 8. `if constexpr` (Not used yet, but useful!)

**Not yet used, but good for template metaprogramming**

```cpp
template<typename T>
void process(T value) {
    if constexpr (std::is_integral_v<T>) {
        // Integer-specific code
    } else if constexpr (std::is_floating_point_v<T>) {
        // Float-specific code
    }
}
```

## 9. Ranges (C++20, not used yet)

**Could be used for text processing**

```cpp
#include <ranges>

// Instead of manual loops
auto tokens = text 
    | std::views::split(' ')
    | std::views::transform(process_token)
    | std::views::filter(is_valid);
```

## 10. Concepts (C++20, not used yet)

**Could improve template constraints**

```cpp
template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

template<Numeric T>
T square(T x) { return x * x; }
```

## C++23 Features to Consider (Future)

### 1. `std::expected<T, E>` - Better Error Handling

**Instead of:**
```cpp
bool load(const std::string& filepath);
```

**Use:**
```cpp
std::expected<Vocabulary, LoadError> load(const std::string& filepath);

// Usage
auto result = vocab.load("model.json");
if (result) {
    // Success: use *result
} else {
    // Error: examine result.error()
}
```

### 2. `std::mdspan` - Multi-dimensional Arrays

**For probability tables:**
```cpp
std::mdspan<double, std::extents<size_t, std::dynamic_extent, std::dynamic_extent>> 
    probabilities(data.data(), vocab_size, context_size);

auto prob = probabilities[token_id, context_id];
```

### 3. Improved Ranges

```cpp
// More elegant text processing
auto words = text 
    | std::views::split(' ')
    | std::views::transform(to_lower)
    | std::views::filter(is_valid)
    | std::ranges::to<std::vector>();
```

### 4. `std::print` - Better Output

**Instead of:**
```cpp
std::cout << "[" << current << "/" << total << "] " << msg << "\n";
```

**Use:**
```cpp
std::print("[{}/{}] {}\n", current, total, msg);
```

## Best Practices Applied

### 1. Type Aliases for Clarity
```cpp
using TokenId = uint32_t;
using Token = std::string;
```

### 2. Smart Pointers - No Manual Memory Management
```cpp
std::unique_ptr<Impl> pimpl_;  // Automatic cleanup
```

### 3. Const Correctness
```cpp
Tokens encode(const std::string& text) const;  // const method
```

### 4. Deleted Copy Constructors
```cpp
Tokenizer(const Tokenizer&) = delete;  // Prevent copying
```

### 5. Default Parameters
```cpp
explicit Tokenizer(const TokenizerConfig& config = TokenizerConfig{});
```

## Learning Resources

- **C++20:** https://en.cppreference.com/w/cpp/20
- **C++23:** https://en.cppreference.com/w/cpp/23
- **Move Semantics:** https://www.cprogramming.com/c++11/rvalue-references-and-move-semantics-in-c++11.html
- **PIMPL:** https://en.cppreference.com/w/cpp/language/pimpl

## Compiler Support

**C++20 Requirements:**
- MSVC 19.29+ (Visual Studio 2019 16.11+)
- GCC 10+
- Clang 10+

**C++23 Requirements:**
- MSVC 19.33+ (Visual Studio 2022 17.3+)
- GCC 11+
- Clang 15+

## IDE Tips

### Visual Studio
- **Intellisense**: Hover over types to see deduced types
- **Go to Definition**: F12 on any symbol
- **Find All References**: Shift+F12

### VS Code
- Install C/C++ extension
- Use Ctrl+Click for navigation
- Ctrl+Space for autocomplete

## Migration Path to C++23

When ready to upgrade:

1. **Update CMakeLists.txt:**
   ```cmake
   set(CMAKE_CXX_STANDARD 23)
   ```

2. **Add `std::expected`:**
   ```cpp
   #include <expected>
   std::expected<Vocabulary, Error> load(const std::string& filepath);
   ```

3. **Use `std::print`:**
   ```cpp
   #include <print>
   std::print("Token: {}\n", token);
   ```

4. **Leverage `std::mdspan`:**
   ```cpp
   #include <mdspan>
   // For multi-dimensional data
   ```

## Questions?

See [ROADMAP.md](ROADMAP.md) for planned C++23 features and their benefits.
