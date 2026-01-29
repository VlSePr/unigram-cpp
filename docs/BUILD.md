# Build Instructions

This document provides detailed instructions for building the Unigram Tokeniser project on different platforms.

## Windows (Visual Studio)

### Prerequisites
- Visual Studio 2019 or later
- CMake 3.20 or later
- Git

### Build Steps

1. **Clone the repository**
   ```cmd
   git clone <repository-url>
   cd UnigramTokeniser
   ```

2. **Generate Visual Studio solution**
   ```cmd
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```

3. **Open in Visual Studio**
   ```cmd
   start UnigramTokeniser.sln
   ```
   Or build from command line:
   ```cmd
   cmake --build . --config Release
   ```

4. **Run tests**
   ```cmd
   ctest -C Release
   ```

### Build Options

- `BUILD_SHARED_LIBS`: Build as DLL (ON by default)
- `BUILD_TESTS`: Build test suite (ON by default)
- `BUILD_PYTHON_BINDINGS`: Build Python bindings (OFF by default)
- `BUILD_JAVA_BINDINGS`: Build Java bindings (OFF by default)

Example with options:
```cmd
cmake .. -G "Visual Studio 17 2022" -DBUILD_PYTHON_BINDINGS=ON
```

## Linux

### Prerequisites
```bash
sudo apt-get update
sudo apt-get install build-essential cmake git
```

### Build Steps

1. **Clone and build**
   ```bash
   git clone <repository-url>
   cd UnigramTokeniser
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   make -j$(nproc)
   ```

2. **Run tests**
   ```bash
   ctest
   ```

3. **Install (optional)**
   ```bash
   sudo make install
   ```

## macOS

### Prerequisites
```bash
brew install cmake
```

### Build Steps

Same as Linux:
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
ctest
```

## Building Python Bindings

### Prerequisites
- Python 3.8 or later
- pip

### Build Steps
```bash
mkdir build && cd build
cmake .. -DBUILD_PYTHON_BINDINGS=ON
cmake --build . --config Release

# Install Python package
cmake --install .
```

### Usage
```python
import unigram
tokenizer = unigram.Tokenizer()
```

## Building Java Bindings

### Prerequisites
- JDK 11 or later

### Build Steps
```bash
mkdir build && cd build
cmake .. -DBUILD_JAVA_BINDINGS=ON
cmake --build . --config Release

# Compile Java classes
javac -d build/java bindings/java/src/main/java/com/unigram/*.java
```

## CMake Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_SHARED_LIBS` | ON | Build shared library (.dll/.so) |
| `BUILD_TESTS` | ON | Build unit tests |
| `BUILD_BENCHMARKS` | OFF | Build performance benchmarks |
| `BUILD_PYTHON_BINDINGS` | OFF | Build Python bindings |
| `BUILD_JAVA_BINDINGS` | OFF | Build Java bindings |
| `CMAKE_BUILD_TYPE` | - | Release or Debug |

## Troubleshooting

### Windows: Missing DLL
If you get "missing DLL" errors, make sure the build output directory is in your PATH:
```cmd
set PATH=%PATH%;C:\path\to\UnigramTokeniser\build\bin
```

### Linux: Library not found
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
```

### CMake version too old
Update CMake:
```bash
pip install --upgrade cmake
```
