# Colab Installation Issue - Root Cause and Fix

## Problem Summary

When installing `unigram-tokeniser` in Google Colab using `pip install .`, the package appeared to install successfully, but importing failed with:

```python
ModuleNotFoundError: No module named 'unigram_tokeniser.unigram'
```

## Root Cause

The issue was in `setup.py`. During the CMake build process:

1. ✅ The C++ shared library (`libunigram_tokeniser.so`) was being compiled correctly
2. ✅ The Python extension module (`unigram.cpython-312-x86_64-linux-gnu.so`) was being built by pybind11
3. ❌ **BUT** only `libunigram_tokeniser.so` was being copied to the installation directory

The `setup.py` had code to search for and copy `libunigram_tokeniser.so`, but it didn't search for or copy the actual **Python extension module** that pybind11 creates.

### What Gets Installed

**Before the Fix:**
```
unigram_tokeniser/
  ├── __init__.py
  └── libunigram_tokeniser.so        ← C++ library only
```

`__init__.py` tries to `from .unigram import Tokenizer` but `unigram` module doesn't exist!

**After the Fix:**
```
unigram_tokeniser/
  ├── __init__.py
  ├── unigram.cpython-312-x86_64-linux-gnu.so    ← Python extension module (NEW!)
  └── libunigram_tokeniser.so                     ← C++ library
```

Now `from .unigram import Tokenizer` works because the `unigram` module exists.

## The Fix

Updated `setup.py` to add explicit code that searches for and copies the Python extension module:

```python
# Copy the Python extension module (the actual pybind11 module)
# This is critical - it's what Python imports!
import glob

# Search for the Python extension in the bindings/python directory
python_ext_patterns = [
    Path(self.build_temp) / "bindings" / "python" / "unigram*.so",
    Path(self.build_temp) / "bindings" / "python" / "unigram*.pyd",
]

ext_found = False
for pattern in python_ext_patterns:
    matching_files = glob.glob(str(pattern))
    for ext_path in matching_files:
        ext_path = Path(ext_path)
        shutil.copy2(str(ext_path), extdir)
        ext_found = True
        print(f"Copied Python extension {ext_path.name} from {ext_path} to {extdir}")
        break
    if ext_found:
        break

if not ext_found:
    print(f"ERROR: Could not find Python extension module!")
    # ... diagnostic output ...
```

## Verification

After installing with the fixed `setup.py`, verify BOTH files are present:

```python
!pip show -f unigram-tokeniser | grep -E "(unigram|libunigram)"
```

**Expected output:**
```
unigram_tokeniser/__init__.py
unigram_tokeniser/unigram.cpython-312-x86_64-linux-gnu.so    ← Critical!
unigram_tokeniser/libunigram_tokeniser.so
```

If you only see `libunigram_tokeniser.so`, the Python extension module is missing.

## Why This Only Affected Colab

The issue likely occurred because:

1. **Different build environments**: Local development (Windows) vs. Colab (Linux) may have different CMake output directory structures
2. **Missing error detection**: The original `setup.py` didn't check if the Python extension module was copied
3. **Implicit vs explicit**: On some systems, CMake's `CMAKE_LIBRARY_OUTPUT_DIRECTORY` might place both files in the right location, but on Colab's Linux environment, the Python extension ended up in a subdirectory

## Testing the Fix

To test the fix in Colab:

```python
# Cell 1: Install with latest code
%cd /content
!rm -rf unigram-cpp
!git clone https://github.com/VlSePr/unigram-cpp.git
%cd unigram-cpp/bindings/python
!pip install --verbose .

# Look for this in the output:
#   "Copied Python extension unigram.cpython-XXX.so from ... to ..."
#   "Copied libunigram_tokeniser.so from ... to ..."

# Verify both files are installed
!pip show -f unigram-tokeniser | grep -E "\.so|\.pyd"

# Restart runtime: Runtime → Restart Runtime

# Cell 2: Import test (after restart)
import unigram_tokeniser
print(f"✓ Success! Version: {unigram_tokeniser.__version__}")

# Quick functional test
config = unigram_tokeniser.TokenizerConfig()
config.vocab_size = 100
trainer = unigram_tokeniser.Trainer(config)
vocab = trainer.train("test " * 50)
print(f"✓ Training works! Vocab size: {vocab.size()}")
```

## Related Files

- `setup.py` - Contains the fix
- `COLAB.md` - Updated troubleshooting section
- `bindings/python/CMakeLists.txt` - Defines the pybind11 module
- `unigram_tokeniser/__init__.py` - Imports from the extension module

## Technical Details

### CMakeLists.txt Configuration

```cmake
pybind11_add_module(unigram_py bindings.cpp)
target_link_libraries(unigram_py PRIVATE unigram::tokeniser)
set_target_properties(unigram_py PROPERTIES OUTPUT_NAME unigram)
```

This creates a Python extension module named `unigram` (not `unigram_py`) that gets compiled to:
- Linux: `unigram.cpython-312-x86_64-linux-gnu.so`
- Windows: `unigram.cp312-win_amd64.pyd`
- macOS: `unigram.cpython-312-darwin.so`

### Import Chain

1. User code: `import unigram_tokeniser`
2. Python loads: `unigram_tokeniser/__init__.py`
3. `__init__.py` does: `from .unigram import Tokenizer, ...`
4. Python looks for: `unigram_tokeniser/unigram.cpython-*.so` ← **This file was missing!**
5. Extension module loads and links: `libunigram_tokeniser.so`

If step 4 fails (file not found), you get: `ModuleNotFoundError: No module named 'unigram_tokeniser.unigram'`

## Commit Message

```
Fix: Python extension module not being installed in Colab

The setup.py was copying libunigram_tokeniser.so but not the Python
extension module (unigram.cpython-*.so) created by pybind11. This caused
imports to fail with "ModuleNotFoundError: No module named 'unigram_tokeniser.unigram'"
when installing via pip in Google Colab.

Added explicit glob search and copy for the Python extension module
in the CMakeBuild.build_extension method. Also added error message
if the extension module is not found after build.

Fixes installation issues in Linux environments where the Python
extension ends up in the build/bindings/python subdirectory.
```
