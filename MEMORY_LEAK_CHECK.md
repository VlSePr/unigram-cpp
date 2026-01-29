# Memory Leak Detection Guide

## Quick Check

Your code already uses **smart pointers** (`std::unique_ptr`), which means **most memory is automatically managed**. This is excellent! 

No raw `new/delete` or `malloc/free` calls were found, which greatly reduces leak risk.

## Methods to Check for Memory Leaks

### Method 1: AddressSanitizer (Recommended for Windows)

Run the provided script:
```bash
.\check_leaks.bat
```

This will:
- Build with AddressSanitizer enabled
- Run a small test
- Report any memory leaks

### Method 2: Visual Studio Memory Leak Detection

Add to your `main.cpp` at the very beginning:

```cpp
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

int main(int argc, char* argv[]) {
    #ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    #endif
    
    // ... rest of code
}
```

Then run in Debug mode:
```bash
cmake --build build --config Debug
.\build\bin\Debug\unigram-cli.exe train --input-dir data --vocab-size 1000
```

### Method 3: Dr. Memory (Free Windows Tool)

1. Download from: https://drmemory.org/
2. Run:
```bash
drmemory.exe -- .\build\bin\Release\unigram-cli.exe train --input data\sample_corpus.txt --vocab-size 1000
```

### Method 4: Windows Performance Toolkit

1. Open Performance Monitor
2. Use "Memory\Available MBytes" counter
3. Watch memory usage during training

## Common Issues to Watch

Even with smart pointers, potential issues:
1. **Circular references** with shared_ptr (not used in your code ✓)
2. **File handles** not closed (check if all ifstream/ofstream close properly)
3. **Large temporary objects** in loops (check trainer loops)
4. **Static/global objects** holding memory

## Current Code Status

✅ **No raw pointers**
✅ **Uses std::unique_ptr for PIMPL idiom**
✅ **STL containers (automatic cleanup)**
✅ **RAII pattern followed**

The code looks clean! But running sanitizers will confirm.
