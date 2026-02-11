# Using Unigram Tokeniser in Google Colab

This guide shows you how to install and use the Unigram Tokeniser in Google Colab notebooks.

## Quick Install (Recommended)

Just run these cells in your Colab notebook:

### Cell 1: Install
```python
# Remove any existing installation (in case you're re-running)
%cd /content
!rm -rf unigram-cpp

# Clone repository
!git clone https://github.com/VlSePr/unigram-cpp.git

# Navigate to Python bindings directory
# Note: If your repo has nested structure, adjust path accordingly
%cd /content/unigram-cpp/bindings/python

# Check we're in the right place
!ls -la
print("Current directory:", !pwd)

# Install (this automatically compiles C++ code using CMake)
# Removed -q flag so you can see build progress and any errors
!pip install .

# Verify installation
!pip show unigram-tokeniser

print("\n" + "="*50)
print("✓ Installation complete!")
print("="*50)
print("⚠️  IMPORTANT: You MUST restart the runtime now:")
print("   1. Go to: Runtime → Restart Runtime")
print("   2. After restart, run Cell 2")
print("="*50)
```

### Cell 2: Import and Test (Run AFTER restarting runtime)
```python
# ⚠️ DID YOU RESTART THE RUNTIME after Cell 1?
# If not, go to: Runtime → Restart Runtime, then come back here

# Try importing
try:
    import unigram_tokeniser
    print("✓ Import successful!")
    print(f"Version: {unigram_tokeniser.__version__}")
    print(f"\nAvailable classes:")
    for name in ['Tokenizer', 'Trainer', 'Vocabulary', 'TokenizerConfig']:
        if hasattr(unigram_tokeniser, name):
            print(f"  ✓ {name}")
except ModuleNotFoundError as e:
    print("✗ Import failed!")
    print("\nDiagnostics:")
    print("=" * 50)
    
    # Check if package is installed
    import subprocess
    result = subprocess.run(['pip', 'show', 'unigram-tokeniser'], 
                          capture_output=True, text=True)
    if result.returncode == 0:
        print("Package IS installed:")
        print(result.stdout)
        print("\n⚠️  You MUST restart the runtime:")
        print("   Runtime → Restart Runtime")
        print("   Then re-run this cell")
    else:
        print("Package is NOT installed!")
        print("Please re-run Cell 1")
    
    raise

# Quick test
config = unigram_tokeniser.TokenizerConfig()
config.vocab_size = 500
config.num_iterations = 2

trainer = unigram_tokeniser.Trainer(config)
vocab = trainer.train("hello world test data " * 100)

tokenizer = unigram_tokeniser.Tokenizer(config)
tokenizer.set_vocabulary(vocab)

tokens = tokenizer.encode("hello world")
print(f"\nTokens: {tokens}")
print(f"✓ Everything works!")
```

## Complete Example

### Training and Using a Tokenizer

```python
import unigram_tokeniser

# 1. Configure
config = unigram_tokeniser.TokenizerConfig()
config.vocab_size = 1000
config.num_iterations = 3
config.byte_fallback = True

# 2. Train from text
corpus = """
The quick brown fox jumps over the lazy dog.
Machine learning is fascinating.
Python makes data science accessible.
""" * 50  # Repeat for sufficient training data

trainer = unigram_tokeniser.Trainer(config)
vocab = trainer.train(corpus)

print(f"Trained vocabulary size: {vocab.size()}")

# 3. Create tokenizer
tokenizer = unigram_tokeniser.Tokenizer(config)
tokenizer.set_vocabulary(vocab)

# 4. Use it
test_sentences = [
    "The quick brown fox",
    "Machine learning is cool",
    "Hello from Google Colab!"
]

for sentence in test_sentences:
    tokens = tokenizer.encode(sentence)
    token_ids = tokenizer.encode_as_ids(sentence)
    decoded = tokenizer.decode(tokens)
    
    print(f"\nInput: {sentence}")
    print(f"Tokens: {tokens}")
    print(f"IDs: {token_ids}")
    print(f"Decoded: {decoded}")
    print(f"✓ Match: {decoded == sentence}")
```

### Training from Files

If you have a text file in Google Drive:

```python
from google.colab import drive
drive.mount('/content/drive')

# Train from your file
trainer = unigram_tokeniser.Trainer(config)
vocab = trainer.train_from_file('/content/drive/MyDrive/corpus.txt')

# Create and use tokenizer
tokenizer = unigram_tokeniser.Tokenizer(config)
tokenizer.set_vocabulary(vocab)

# Save model
tokenizer.save('/content/drive/MyDrive/my_tokenizer.json')

# Load it later
loaded = unigram_tokeniser.Tokenizer.from_file('/content/drive/MyDrive/my_tokenizer.json')
```

### Training from Wikipedia or Downloaded Data

```python
# Download sample data
!wget https://raw.githubusercontent.com/karpathy/char-rnn/master/data/tinyshakespeare/input.txt

# Train on it
config = unigram_tokeniser.TokenizerConfig()
config.vocab_size = 2000
config.num_iterations = 4

trainer = unigram_tokeniser.Trainer(config)
vocab = trainer.train_from_file('input.txt')

print(f"Vocabulary size: {vocab.size()}")

# Use the trained tokenizer
tokenizer = unigram_tokeniser.Tokenizer(config)
tokenizer.set_vocabulary(vocab)

sample = "To be or not to be, that is the question."
tokens = tokenizer.encode(sample)
print(f"Tokens: {tokens}")
```

## Troubleshooting

### If Installation Fails

**Error: "destination path already exists"**

The repository was cloned in a previous run. Remove it first:

```python
%cd /content
!rm -rf unigram-cpp
!git clone https://github.com/VlSePr/unigram-cpp.git
%cd unigram-cpp/bindings/python
!pip install .
```

**Error: "Neither 'setup.py' nor 'pyproject.toml' found"**

You're in the wrong directory. Make sure you're in `bindings/python`:

```python
%cd /content/unigram-cpp/bindings/python
!pip install .
```

**Error: "ModuleNotFoundError: No module named 'unigram'" after installation**

This is the **most common issue** in Colab. The installation completed successfully, but Python's runtime doesn't know about the new module yet.

**✓ Solution (Simple - ALWAYS WORKS):**

1. After Cell 1 completes, **IMMEDIATELY** restart the runtime:
   - Click: `Runtime → Restart Runtime`
   - Confirm the restart
2. Run Cell 2 (the import cell) - it will now work

**Why does this happen?**
- Colab keeps Python running in the background
- When you `pip install`, it adds files to disk
- But the running Python interpreter doesn't automatically rescan for new modules
- Restarting the runtime = starting fresh with the new module available

**Error: "ModuleNotFoundError: No module named 'unigram_tokeniser.unigram'"**

This is a **different issue** - the package installs but the Python extension module is missing.

**Diagnosis:**
```python
# Check what files were actually installed
!pip show -f unigram-tokeniser
```

If you see:
```
Files:
  unigram_tokeniser/__init__.py
  unigram_tokeniser/libunigram_tokeniser.so
```

But **missing** something like `unigram_tokeniser/unigram.cpython-312-x86_64-linux-gnu.so`, then the Python extension module didn't get installed. The package needs BOTH files:
- `libunigram_tokeniser.so` - the core C++ library
- `unigram.cpython-*.so` - the Python binding module (THIS IS WHAT'S MISSING!)

**✓ Solution:**

This was a bug in setup.py that has been fixed. Update to the latest version:

```python
%cd /content
!rm -rf unigram-cpp
!git clone https://github.com/VlSePr/unigram-cpp.git
%cd unigram-cpp/bindings/python

# Install with verbose output to see what's happening
!pip install --force-reinstall --no-cache-dir --verbose .

# Verify BOTH files are present
!pip show -f unigram-tokeniser | grep -E "(unigram|libunigram)"

# You should see BOTH:
#   unigram_tokeniser/unigram.cpython-XXX-linux-gnu.so    ← This is the Python module!
#   unigram_tokeniser/libunigram_tokeniser.so             ← This is the C++ library
```

If you still only see `libunigram_tokeniser.so`, check the build output for errors. The Python extension module should be built during the CMake compilation step.

**Alternative diagnostic steps** (if restart doesn't work):

```python
# Step 1: Verify package is installed
!pip show unigram-tokeniser
# You should see: Name: unigram-tokeniser, Version: x.x.x

# Step 2: Find where it's installed
!pip show -f unigram-tokeniser | grep Location

# Step 3: Check if the .so file exists
!find /usr/local/lib -name "*unigram*.so" 2>/dev/null

# Step 4: Try manual import with explicit path
import sys
sys.path.insert(0, '/usr/local/lib/python3.10/site-packages')
import unigram

# If Step 4 works, the issue is just sys.path
# If Step 4 fails, the installation had errors - check Cell 1 output
```

**If nothing works:**
```python
# Nuclear option: Fresh install with full output
%cd /content
!rm -rf unigram-cpp
!git clone https://github.com/VlSePr/unigram-cpp.git
%cd unigram-cpp/bindings/python
!pip install --force-reinstall --no-cache-dir --verbose .
# Look for any errors in the output above
# Then restart runtime and try again
```

**CMake Issues**

Sometimes Colab might be missing CMake or have an old version. Try:

```python
# Update CMake
!pip install cmake --upgrade

# Try installing again
%cd /content/unigram-cpp/bindings/python
!pip install --force-reinstall --no-cache-dir .
```

### Check Installation

```python
import unigram
print(f"Version: {unigram_tokeniser.__version__}")
print(f"Available: {dir(unigram)}")
```

### Memory Issues

If training on large corpora in Colab:

```python
# Use smaller vocab size
config.vocab_size = 1000  # Instead of 32000

# Use fewer iterations
config.num_iterations = 2  # Instead of 5

# Or train on a subset of your data
with open('large_file.txt', 'r') as f:
    corpus = f.read(10_000_000)  # Read only 10MB
vocab = trainer.train(corpus)
```

## Performance Note

Google Colab provides free GPU access, but tokenizer training runs on CPU. The C++ implementation is still very fast:
- Training 1000 vocab on 1MB text: ~3-5 seconds
- Tokenizing 10,000 sentences: <1 second

## Saving Your Work

Always save your trained models to Google Drive:

```python
from google.colab import drive
drive.mount('/content/drive')

# Save tokenizer
%cd /content
!rm -rf unigram-cpp  # Remove if exists
tokenizer.save('/content/drive/MyDrive/tokenizer.json')

# Later, in a new session:
tokenizer = unigram_tokeniser.Tokenizer.from_file('/content/drive/MyDrive/tokenizer.json')
```

## Why pip install Works

The Python bindings' `pyproject.toml` configuration (using the setuptools PEP 517 build backend) automatically:
1. ✓ Detects your system (Linux in Colab's case)
2. ✓ Runs CMake to configure the build
3. ✓ Compiles the C++ code with the right compiler flags
4. ✓ Creates the Python extension module
5. ✓ Installs it so you can `import unigram`

You don't need to manually run CMake commands - it's all handled for you!

## Complete Colab Notebook Template

**IMPORTANT:** You MUST restart the runtime after Cell 1, before running Cell 2!

```python
# ============================================
# Cell 1: Install
# ============================================
%cd /content
!rm -rf unigram-cpp
!git clone https://github.com/VlSePr/unigram-cpp.git
%cd unigram-cpp/bindings/python
!pip install .
!pip show unigram-tokeniser

print("\n" + "="*50)
print("✓ Installation complete!")
print("="*50)
print("⚠️  NOW: Runtime → Restart Runtime")
print("     Then continue with Cell 2")
print("="*50)

# ============================================
# ⚠️⚠️⚠️ STOP HERE - RESTART RUNTIME ⚠️⚠️⚠️
# Go to: Runtime → Restart Runtime
# Then run Cell 2 below
# ============================================

# ============================================
# Cell 2: Import and Configure (Run AFTER restarting runtime)
# ============================================
import unigram
print("✓ Import successful!")

config = unigram_tokeniser.TokenizerConfig()
config.vocab_size = 1000
config.num_iterations = 3
config.byte_fallback = True

# ============================================
# Cell 3: Train
# ============================================
corpus = "Your training text here " * 100
trainer = unigram_tokeniser.Trainer(config)
vocab = trainer.train(corpus)
print(f"Vocabulary size: {vocab.size()}")

# ============================================
# Cell 4: Create Tokenizer
# ============================================
tokenizer = unigram_tokeniser.Tokenizer(config)
tokenizer.set_vocabulary(vocab)

# ============================================
# Cell 5: Use It
# ============================================
text = "Hello, world!"
tokens = tokenizer.encode(text)
token_ids = tokenizer.encode_as_ids(text)
decoded = tokenizer.decode(tokens)

print(f"Input: {text}")
print(f"Tokens: {tokens}")
print(f"IDs: {token_ids}")
print(f"Decoded: {decoded}")

# ============================================
# Cell 6: Save (Optional)
# ============================================
from google.colab import drive
drive.mount('/content/drive')
tokenizer.save('/content/drive/MyDrive/my_tokenizer.json')
```

That's it! Happy tokenizing in Colab! 🚀

