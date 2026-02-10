# Unigram Tokeniser Python Bindings

Python bindings for the Unigram Tokeniser C++ library.

## Installation

### Option 1: Install from source (pip)

```bash
# Navigate to the python bindings directory
cd bindings/python

# Install in development mode (recommended for development)
pip install -e .

# Or install normally
pip install .

# With development dependencies
pip install -e ".[dev]"
```

### Option 2: Build with CMake

```bash
# From the project root
mkdir build
cd build
cmake .. -DBUILD_PYTHON_BINDINGS=ON
cmake --build . --config Release

# The module will be built in build/lib/
# Add it to PYTHONPATH or copy to site-packages
```

### Prerequisites

- Python 3.8 or higher
- CMake 3.20 or higher
- C++20 compatible compiler
- pybind11 (automatically fetched during build)

## Usage

```python
import unigram

# Create and configure tokenizer
config = unigram.TokenizerConfig()
config.vocab_size = 32000

# Train a tokenizer
trainer = unigram.Trainer(config)
vocab = trainer.train_from_file("corpus.txt")

# Create tokenizer with trained vocabulary
tokenizer = unigram.Tokenizer(config)
tokenizer.set_vocabulary(vocab)

# Use the tokenizer
tokens = tokenizer.encode("Hello, world!")
token_ids = tokenizer.encode_as_ids("Hello, world!")
text = tokenizer.decode(tokens)

# Save and load
tokenizer.save("model.json")
loaded_tokenizer = unigram.Tokenizer.from_file("model.json")
```

## Features

- Full access to C++ library functionality
- Pythonic API with type hints
- Efficient native performance
- Compatible with Python 3.8+

## Development

### Running Tests

```bash
# Install with dev dependencies
pip install -e ".[dev]"

# Run tests
pytest tests/

# Run with coverage
pytest tests/ --cov=unigram --cov-report=html
```

### Building Distribution

```bash
# Install build tool
pip install build

# Build wheel and sdist
python -m build

# The built packages will be in dist/
```

### Type Checking

```bash
mypy examples/
```

## Examples

See the `examples/` directory for usage examples:

- [`basic_usage.py`](examples/basic_usage.py) - Complete workflow demonstration
- [`train_from_file.py`](examples/train_from_file.py) - Training from text files

## API Documentation

### TokenizerConfig

Configuration for tokenizer and trainer.

```python
config = unigram.TokenizerConfig()
config.vocab_size = 32000          # Target vocabulary size
config.num_iterations = 4          # Training iterations
config.byte_fallback = True        # Use byte fallback for unknown chars
config.add_bos_token = False       # Add beginning-of-sequence token
config.add_eos_token = False       # Add end-of-sequence token
config.shrinking_factor = 0.75     # Vocabulary shrinking factor
config.min_frequency = 2           # Minimum token frequency
```

### Trainer

Train a tokenizer from text data.

```python
trainer = unigram.Trainer(config)

# Train from string
vocab = trainer.train(corpus_text)

# Train from file
vocab = trainer.train_from_file("corpus.txt")

# With progress callback
def callback(iteration, message):
    print(f"Iteration {iteration}: {message}")
    
trainer.set_progress_callback(callback)
```

### Tokenizer

Tokenize and detokenize text.

```python
tokenizer = unigram.Tokenizer(config)
tokenizer.set_vocabulary(vocab)

# Encode text to tokens
tokens = tokenizer.encode("Hello, world!")
# Returns: ['Hello', ',', ' world', '!']

# Encode text to token IDs
token_ids = tokenizer.encode_as_ids("Hello, world!")
# Returns: [234, 15, 456, 23]

# Decode tokens back to text
text = tokenizer.decode(tokens)
text = tokenizer.decode(token_ids)

# Save and load
tokenizer.save("model.json")
tokenizer.load("model.json")

# Or use static method
tokenizer = unigram.Tokenizer.from_file("model.json")
```

### Vocabulary

Direct vocabulary manipulation (advanced usage).

```python
vocab = unigram.Vocabulary()

# Add tokens
vocab.add_token("hello", -1.5)  # token, log_probability

# Query
token_id = vocab.get_id("hello")
token = vocab.get_token(token_id)
score = vocab.get_score(token_id)

# Check
if vocab.contains("hello"):
    print(f"Size: {len(vocab)}")

# Save/Load
vocab.save("vocab.json")
vocab.load("vocab.json")
```

## Troubleshooting

### Import Error

If you get `ImportError: No module named 'unigram'`:

1. Make sure you've installed the package: `pip install -e .`
2. Or add the build directory to PYTHONPATH: `export PYTHONPATH=/path/to/build/lib:$PYTHONPATH`

### Build Errors

If CMake can't find Python:
```bash
pip install --upgrade pip setuptools wheel
cmake .. -DPYTHON_EXECUTABLE=$(which python3)
```

If pybind11 is not found:
```bash
pip install pybind11
```

## License

Same as the main UnigramTokeniser project.
