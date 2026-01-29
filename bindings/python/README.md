# Unigram Tokeniser Python Bindings

Python bindings for the Unigram Tokeniser C++ library.

## Installation

```bash
# Build and install
cd build
cmake .. -DBUILD_PYTHON_BINDINGS=ON
cmake --build . --config Release
cmake --install .
```

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
