# Building and Installing Python Bindings

This guide walks you through building and installing the Python bindings for UnigramTokeniser.

## Prerequisites

Make sure you have:
- Python 3.8 or higher
- pip (Python package installer)
- CMake 3.20 or higher
- C++20 compatible compiler (MSVC 2019+ on Windows, GCC 10+/Clang 10+ on Linux/macOS)

Check your versions:
```bash
python --version
pip --version
cmake --version
```

## Method 1: Install with pip (Recommended)

This method automatically handles the CMake build process.

### Step 1: Navigate to the Python bindings directory

```bash
cd bindings/python
```

### Step 2: Install the package

For regular installation:
```bash
pip install .
```

For development (editable mode):
```bash
pip install -e .
```

With development dependencies (for running tests):
```bash
pip install -e ".[dev]"
```

### Step 3: Verify installation

```python
python -c "import unigram; print(unigram.__version__)"
```

## Method 2: Build with CMake

If you prefer to build with CMake directly:

### Step 1: Create build directory

From the project root:
```bash
mkdir build
cd build
```

### Step 2: Configure CMake

```bash
cmake .. -DBUILD_PYTHON_BINDINGS=ON -DCMAKE_BUILD_TYPE=Release
```

On Windows with Visual Studio:
```bash
cmake .. -DBUILD_PYTHON_BINDINGS=ON -G "Visual Studio 17 2022"
```

### Step 3: Build

```bash
cmake --build . --config Release
```

### Step 4: Use the module

The compiled module will be in `build/lib/` (or `build/lib/Release/` on Windows).

Add it to your Python path:
```bash
# Linux/macOS
export PYTHONPATH=/path/to/build/lib:$PYTHONPATH

# Windows (PowerShell)
$env:PYTHONPATH = "C:\path\to\build\lib;$env:PYTHONPATH"
```

Or copy it to your Python site-packages:
```bash
python -c "import site; print(site.getsitepackages())"
# Copy the .pyd/.so file to one of these directories
```

## Testing Your Installation

### Quick test:

```python
python -c "import unigram; print('Success!'); print(f'Version: {unigram.__version__}')"
```

### Run the basic example:

```bash
cd bindings/python
python examples/basic_usage.py
```

### Run the test suite:

```bash
# Install pytest if needed
pip install pytest

# Run tests
cd bindings/python
pytest tests/ -v
```

## Troubleshooting

### Problem: `ImportError: No module named 'unigram'`

**Solution 1:** Make sure you've installed the package:
```bash
pip install -e bindings/python/
```

**Solution 2:** Add the build directory to PYTHONPATH (if using CMake method):
```bash
export PYTHONPATH=/path/to/build/lib:$PYTHONPATH
```

### Problem: CMake can't find Python

**Solution:** Specify Python executable explicitly:
```bash
cmake .. -DBUILD_PYTHON_BINDINGS=ON -DPYTHON_EXECUTABLE=$(which python3)
```

### Problem: pybind11 not found

**Solution:** Install pybind11 (it should auto-fetch, but just in case):
```bash
pip install pybind11
```

### Problem: Compiler errors

**Solution:** Make sure you have a C++20 compliant compiler:
- Windows: Visual Studio 2019 or later
- Linux: GCC 10+ or Clang 10+
- macOS: Xcode 12+ or Clang 10+

### Problem: Tests fail with module not found

**Solution:** Install the package in editable mode:
```bash
cd bindings/python
pip install -e ".[dev]"
pytest tests/ -v
```

## Development Workflow

If you're developing the bindings:

1. **Make changes** to `bindings.cpp` or header files

2. **Rebuild**:
   ```bash
   # If using pip install -e .
   pip install -e . --force-reinstall --no-deps
   
   # If using CMake
   cmake --build build --config Release
   ```

3. **Test**:
   ```bash
   pytest bindings/python/tests/ -v
   ```

4. **Format code** (Python files):
   ```bash
   black bindings/python/
   ```

## Next Steps

- Read the [Python bindings README](README.md) for API documentation
- Try the examples in `examples/`
- Check out the main [project README](../../README.md)

## Building a Wheel for Distribution

To create a distributable wheel:

```bash
cd bindings/python
pip install build
python -m build

# The wheel will be in dist/
# Install it with: pip install dist/*.whl
```
