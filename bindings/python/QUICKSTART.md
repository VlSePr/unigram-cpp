# Python Bindings for Unigram Tokeniser - Quick Start

## Installation

From the `bindings/python` directory:

```bash
pip install -e .
```

## Quick Example

```python
import unigram

# Configure the tokenizer
config = unigram.TokenizerConfig()
config.vocab_size = 1000
config.num_iterations = 3

# Train on some text
trainer = unigram.Trainer(config)
corpus = "your training text here " * 100
vocab = trainer.train(corpus)

# Create tokenizer
tokenizer = unigram.Tokenizer(config)
tokenizer.set_vocabulary(vocab)

# Use it!
tokens = tokenizer.encode("Hello, world!")
token_ids = tokenizer.encode_as_ids("Hello, world!")
text = tokenizer.decode(tokens)

print(f"Tokens: {tokens}")
print(f"IDs: {token_ids}")
print(f"Decoded: {text}")

# Save for later
tokenizer.save("my_model.json")

# Load it back
loaded = unigram.Tokenizer.from_file("my_model.json")
```

## Run Examples

```bash
# Basic usage demonstration
python examples/basic_usage.py

# Train from a file
python examples/train_from_file.py ../../../data/alice-in-wonderland trained_model.json
```

## Run Tests

```bash
pip install pytest
pytest tests/ -v
```

## Next Steps

- Check out the full [README.md](README.md) for detailed API documentation
- Explore the `examples/` directory
- Read the main project documentation in the repository root
