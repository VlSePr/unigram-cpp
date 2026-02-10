# Using Unigram Tokeniser in Google Colab

This guide shows you how to install and use the Unigram Tokeniser in Google Colab notebooks.

## Quick Install (Recommended)

Just run these cells in your Colab notebook:

### Cell 1: Install
```python
# Remove any existing installation (in case you're re-running)
!rm -rf unigram-cpp

# Clone repository
!git clone https://github.com/VlSePr/unigram-cpp.git

# Navigate to Python bindings directory
%cd unigram-cpp/bindings/python

# Install (this automatically compiles C++ code using CMake)
!pip install -q .

print("✓ Installation complete!")
```

### Alternative: One-Line Install
```python
# If repo already exists, just reinstall
%cd /content
!rm -rf unigram-cpp && git clone https://github.com/VlSePr/unigram-cpp.git && cd unigram-cpp/bindings/python && pip install -q .
```

### Cell 2: Test Installation
```python
import unigram

# Quick test
config = unigram.TokenizerConfig()
config.vocab_size = 500
config.num_iterations = 2

trainer = unigram.Trainer(config)
vocab = trainer.train("hello world test data " * 100)

tokenizer = unigram.Tokenizer(config)
tokenizer.set_vocabulary(vocab)

tokens = tokenizer.encode("hello world")
print(f"Tokens: {tokens}")
print(f"✓ Everything works!")
```

## Complete Example

### Training and Using a Tokenizer

```python
import unigram

# 1. Configure
config = unigram.TokenizerConfig()
config.vocab_size = 1000
config.num_iterations = 3
config.byte_fallback = True

# 2. Train from text
corpus = """
The quick brown fox jumps over the lazy dog.
Machine learning is fascinating.
Python makes data science accessible.
""" * 50  # Repeat for sufficient training data

trainer = unigram.Trainer(config)
vocab = trainer.train(corpus)

print(f"Trained vocabulary size: {vocab.size()}")

# 3. Create tokenizer
tokenizer = unigram.Tokenizer(config)
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
trainer = unigram.Trainer(config)
vocab = trainer.train_from_file('/content/drive/MyDrive/corpus.txt')

# Create and use tokenizer
tokenizer = unigram.Tokenizer(config)
tokenizer.set_vocabulary(vocab)

# Save model
tokenizer.save('/content/drive/MyDrive/my_tokenizer.json')

# Load it later
loaded = unigram.Tokenizer.from_file('/content/drive/MyDrive/my_tokenizer.json')
```

### Training from Wikipedia or Downloaded Data

```python
# Download sample data
!wget https://raw.githubusercontent.com/karpathy/char-rnn/master/data/tinyshakespeare/input.txt

# Train on it
config = unigram.TokenizerConfig()
config.vocab_size = 2000
config.num_iterations = 4

trainer = unigram.Trainer(config)
vocab = trainer.train_from_file('input.txt')

print(f"Vocabulary size: {vocab.size()}")

# Use the trained tokenizer
tokenizer = unigram.Tokenizer(config)
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
print(f"Version: {unigram.__version__}")
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
tokenizer = unigram.Tokenizer.from_file('/content/drive/MyDrive/tokenizer.json')
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

```python
# ============================================
# Cell 1: Install
# ============================================
!git clone https://github.com/VlSePr/unigram-cpp.git
%cd unigram-cpp/bindings/python
!pip install -q .

# ============================================
# Cell 2: Import and Configure
# ============================================
import unigram

config = unigram.TokenizerConfig()
config.vocab_size = 1000
config.num_iterations = 3
config.byte_fallback = True

# ============================================
# Cell 3: Train
# ============================================
corpus = "Your training text here " * 100
trainer = unigram.Trainer(config)
vocab = trainer.train(corpus)
print(f"Vocabulary size: {vocab.size()}")

# ============================================
# Cell 4: Create Tokenizer
# ============================================
tokenizer = unigram.Tokenizer(config)
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
