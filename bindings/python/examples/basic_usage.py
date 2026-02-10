"""
Basic usage example for the Unigram Tokeniser Python bindings
"""
import unigram


def main():
    print("=" * 60)
    print("Unigram Tokeniser - Basic Usage Example")
    print("=" * 60)
    print()
    
    # Create configuration
    print("1. Creating tokenizer configuration...")
    config = unigram.TokenizerConfig()
    config.vocab_size = 300
    config.num_iterations = 3
    config.byte_fallback = True
    print(f"   Vocabulary size: {config.vocab_size}")
    print(f"   Training iterations: {config.num_iterations}")
    print()
    
    # Create training corpus
    print("2. Preparing training corpus...")
    corpus = """
    The quick brown fox jumps over the lazy dog.
    Python is a high-level programming language.
    Machine learning models require large amounts of training data.
    Natural language processing enables computers to understand human language.
    Tokenization is the process of breaking text into smaller units called tokens.
    """ * 10  # Repeat for sufficient training data
    print(f"   Corpus length: {len(corpus)} characters")
    print()
    
    # Train the tokenizer
    print("3. Training tokenizer...")
    trainer = unigram.Trainer(config)
    
    # Set up progress callback
    def progress_callback(iteration, message):
        print(f"   Iteration {iteration}: {message}")
    
    trainer.set_progress_callback(progress_callback)
    vocab = trainer.train(corpus)
    print(f"   Training complete! Vocabulary size: {vocab.size()}")
    print()
    
    # Create tokenizer with trained vocabulary
    print("4. Creating tokenizer...")
    tokenizer = unigram.Tokenizer(config)
    tokenizer.set_vocabulary(vocab)
    print("   Tokenizer ready!")
    print()
    
    # Test encoding
    print("5. Testing tokenization...")
    test_sentences = [
        "The quick brown fox jumps over the lazy dog.",
        "Hello, world!",
        "Machine learning is fascinating.",
    ]
    
    for sentence in test_sentences:
        print(f"\n   Input: {sentence}")
        
        # Encode to tokens
        tokens = tokenizer.encode(sentence)
        print(f"   Tokens: {tokens}")
        
        # Encode to IDs
        token_ids = tokenizer.encode_as_ids(sentence)
        print(f"   Token IDs: {token_ids}")
        
        # Decode back
        decoded = tokenizer.decode(tokens)
        print(f"   Decoded: {decoded}")
        
        # Verify roundtrip
        if decoded == sentence:
            print("   ✓ Roundtrip successful!")
        else:
            print("   ✗ Roundtrip failed!")
    
    print()
    
    # Save the model
    print("6. Saving model...")
    model_path = "example_model.json"
    tokenizer.save(model_path)
    print(f"   Model saved to: {model_path}")
    print()
    
    # Load the model
    print("7. Loading model...")
    loaded_tokenizer = unigram.Tokenizer.from_file(model_path)
    print("   Model loaded successfully!")
    print()
    
    # Test loaded model
    print("8. Testing loaded model...")
    test_text = "Testing the loaded tokenizer."
    tokens1 = tokenizer.encode(test_text)
    tokens2 = loaded_tokenizer.encode(test_text)
    
    print(f"   Input: {test_text}")
    print(f"   Original tokenizer: {tokens1}")
    print(f"   Loaded tokenizer: {tokens2}")
    
    if tokens1 == tokens2:
        print("   ✓ Models match perfectly!")
    else:
        print("   ✗ Models don't match!")
    
    print()
    print("=" * 60)
    print("Example completed successfully!")
    print("=" * 60)


if __name__ == "__main__":
    main()
