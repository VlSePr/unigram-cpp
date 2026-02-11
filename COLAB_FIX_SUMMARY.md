# Fixes for Colab Installation Issue

## Problem
When running `pip show -f unigram-tokeniser` in Colab, the Python extension module was missing:
- ❌ Missing: `unigram_tokeniser/unigram.cpython-312-x86_64-linux-gnu.so` (Python binding module)
- ✅ Present: `unigram_tokeniser/libunigram_tokeniser.so` (C++ shared library) 
- ✅ Present: `unigram_tokeniser/__init__.py`

The `__init__.py` tries to `from .unigram import ...` but the `unigram` module file doesn't exist, causing:
```
ModuleNotFoundError: No module named 'unigram_tokeniser.unigram'
```

## Root Cause
The `setup.py` had code to search for and copy `libunigram_tokeniser.so`, but it **did not search for or copy the Python extension module** that pybind11 creates. This Python extension module is what `__init__.py` tries to import.

The build process creates TWO separate files:
1. `libunigram_tokeniser.so` - the C++ shared library (was being copied ✅)
2. `unigram.cpython-*.so` - the Python binding module created by pybind11 (was NOT being copied ❌)

## Fixes Applied

### 1. Enhanced setup.py (`bindings/python/setup.py`)
**Changes:**
- ✅ Added explicit search and copy for the Python extension module
- ✅ Searches in `build/temp.DIR/bindings/python/unigram*.so`
- ✅ Searches for both `.so` (Linux) and `.pyd` (Windows) extensions
- ✅ Prints diagnostic info showing what's being copied
- ✅ Shows error message if Python extension module is not found
- ✅ Lists build directory contents for debugging if module is missing

**The fix now:**
1. Builds the C++ library and Python extension via CMake
2. Searches for and copies `libunigram_tokeniser.so` (existing functionality)
3. **NEW:** Searches for and copies `unigram.cpython-*.so` (the Python module!)
4. Prints confirmation when both files are copied
5. Shows helpful diagnostics if either file is missing

### 2. Updated Tests (`bindings/python/tests/test_tokenizer.py`)
**Changes:**
- Updated imports from `import unigram` to `import unigram_tokeniser`
- All test classes now use correct package name
- Tests are compatible with the new package structure

### 3. Added Diagnostic Script (`bindings/python/colab_debug.py`)
**Usage in Colab:**
```python
%cd /content/unigram-cpp/bindings/python
!python colab_debug.py
```

**This script checks:**
- ✓ Python version and platform
- ✓ Package installation status
- ✓ Files present in package directory
- ✓ Extension module can be imported
- ✓ Available API classes and functions
- ✓ Basic functionality (create objects, check constants)

## Testing Locally

### Clean Rebuild:
```powershell
cd bindings\python
Remove-Item -Recurse -Force build, dist, *.egg-info -ErrorAction SilentlyContinue
& "python" -m pip install . --force-reinstall --no-cache-dir
```

### Verify Installation:
```powershell
python -m pip show -f unigram-tokeniser
```

**Expected files:**
```
Files:
  unigram_tokeniser/__init__.py
  unigram_tokeniser/__pycache__/__init__.cpython-312.pyc
  unigram_tokeniser/unigram.cp312-win_amd64.pyd         (Windows)
  unigram_tokeniser/unigram.cpython-312-x86_64-linux-gnu.so  (Linux)
  unigram_tokeniser/unigram_tokeniser.dll                (Windows)
  unigram_tokeniser/libunigram_tokeniser.so              (Linux)
  unigram_tokeniser-1.0.0.dist-info/...
```

### Test Import:
```python
import unigram_tokeniser

# Should work now
vocab = unigram_tokeniser.Vocabulary()
config = unigram_tokeniser.TokenizerConfig()
print(f"✓ Version: {unigram_tokeniser.__version__}")
```

## For Colab Users

### Installation:
```bash
%cd /content
!rm -rf unigram-cpp
!git clone https://github.com/VlSePr/unigram-cpp.git
%cd /content/unigram-cpp/bindings/python

# Install with verbose output to see any errors
!pip install . -v

# Check installation
!pip show -f unigram-tokeniser

# Run diagnostics
!python colab_debug.py
```

### If Build Fails in Colab:
```bash
# Install build dependencies
!apt-get update
!apt-get install -y build-essential cmake

# Try again
!pip install . --force-reinstall --no-cache-dir -v 2>&1 | tee build.log
```

## Next Steps

1. **Commit all changes:**
   ```bash
   git add bindings/python/setup.py
   git add bindings/python/tests/test_tokenizer.py
   git add bindings/python/colab_debug.py
   git add bindings/python/unigram_tokeniser/__init__.py
   git commit -m "Fix: Improve Linux/Colab installation - search multiple paths for shared library"
   git push
   ```

2. **Test in Colab:**
   - After pushing, test the installation in a fresh Colab notebook
   - Run the diagnostic script to verify all files are present
   - Report any issues with the build.log output

3. **Update Documentation:**
   - COLAB.md needs updating to use `import unigram_tokeniser` instead of `import unigram`
   - Add troubleshooting section with diagnostic script

## Debug Output

When the shared library is found, you'll see:
```
Copied libunigram_tokeniser.so from /path/to/build/lib/libunigram_tokeniser.so to /path/to/site-packages/unigram_tokeniser
```

If not found, you'll see:
```
Warning: Could not find libunigram_tokeniser.so in expected locations:
  - /build/path/bin/libunigram_tokeniser.so
  - /build/path/lib/libunigram_tokeniser.so
  - /build/path/src/libunigram_tokeniser.so

Contents of build directory /build/temp.linux-x86_64-cpython-312:
  bin/
    unigram.cpython-312-x86_64-linux-gnu.so
  lib/
    libunigram_tokeniser.so  ← This shows where it actually is!
```

This helps identify the correct path to add to the search list.
