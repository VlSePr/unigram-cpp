"""
Simple script to verify the Python bindings work after installation.
Run this after installing: pip install -e .
"""
def test_import():
    """Test that we can import the module"""
    try:
        import unigram
        print("✓ Successfully imported unigram module")
        print(f"  Version: {unigram.__version__}")
        return True
    except ImportError as e:
        print(f"✗ Failed to import unigram: {e}")
        return False


def test_constants():
    """Test that constants are available"""
    try:
        import unigram
        assert hasattr(unigram, 'UNK_TOKEN')
        assert hasattr(unigram, 'BOS_TOKEN')
        assert unigram.UNK_TOKEN == "<unk>"
        print("✓ Constants are accessible")
        return True
    except Exception as e:
        print(f"✗ Constants test failed: {e}")
        return False


def test_basic_functionality():
    """Test basic tokenizer functionality"""
    try:
        import unigram
        
        # Create config
        config = unigram.TokenizerConfig()
        config.vocab_size = 100
        config.num_iterations = 2
        
        # Train
        trainer = unigram.Trainer(config)
        corpus = "hello world test " * 50
        vocab = trainer.train(corpus)
        
        # Create tokenizer
        tokenizer = unigram.Tokenizer(config)
        tokenizer.set_vocabulary(vocab)
        
        # Test encoding
        tokens = tokenizer.encode("hello world")
        token_ids = tokenizer.encode_as_ids("hello world")
        
        # Test decoding
        decoded = tokenizer.decode(tokens)
        
        print("✓ Basic functionality works")
        print(f"  Vocabulary size: {vocab.size()}")
        print(f"  Test text: 'hello world'")
        print(f"  Tokens: {tokens}")
        print(f"  Token IDs: {token_ids}")
        print(f"  Decoded: '{decoded}'")
        return True
    except Exception as e:
        print(f"✗ Basic functionality test failed: {e}")
        import traceback
        traceback.print_exc()
        return False


def main():
    print("=" * 60)
    print("Python Bindings Verification")
    print("=" * 60)
    print()
    
    results = []
    
    print("1. Testing import...")
    results.append(test_import())
    print()
    
    if results[-1]:  # Only continue if import succeeded
        print("2. Testing constants...")
        results.append(test_constants())
        print()
        
        print("3. Testing basic functionality...")
        results.append(test_basic_functionality())
        print()
    
    print("=" * 60)
    if all(results):
        print("✓ All tests passed! Python bindings are working correctly.")
    else:
        print("✗ Some tests failed. Please check the error messages above.")
        print("\nMake sure you've installed the package:")
        print("  cd bindings/python")
        print("  pip install -e .")
    print("=" * 60)
    
    return 0 if all(results) else 1


if __name__ == "__main__":
    exit(main())
