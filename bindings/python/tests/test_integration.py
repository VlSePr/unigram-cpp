"""
Integration tests for the Unigram Tokenizer
"""
import pytest
import tempfile
import os

try:
    import unigram
except ImportError:
    pytest.skip("unigram module not built", allow_module_level=True)


class TestEndToEndWorkflow:
    def test_complete_workflow(self):
        """Test the complete workflow from training to tokenization"""
        # Create a corpus
        corpus = """
        The quick brown fox jumps over the lazy dog.
        Python is a high-level programming language.
        Machine learning models require large amounts of training data.
        Natural language processing enables computers to understand human language.
        """ * 20  # Repeat for sufficient training data
        
        # Configure trainer
        config = unigram.TokenizerConfig()
        config.vocab_size = 300
        config.num_iterations = 3
        config.byte_fallback = True
        
        # Train tokenizer
        trainer = unigram.Trainer(config)
        vocab = trainer.train(corpus)
        
        # Verify vocabulary was created
        assert vocab.size() > 0
        assert vocab.size() <= config.vocab_size + 10  # Allow some special tokens
        
        # Create tokenizer with trained vocabulary
        tokenizer = unigram.Tokenizer(config)
        tokenizer.set_vocabulary(vocab)
        
        # Test encoding
        test_sentence = "The quick brown fox jumps over the lazy dog."
        tokens = tokenizer.encode(test_sentence)
        token_ids = tokenizer.encode_as_ids(test_sentence)
        
        assert len(tokens) > 0
        assert len(token_ids) > 0
        assert len(tokens) == len(token_ids)
        
        # Test decoding
        decoded_from_tokens = tokenizer.decode(tokens)
        decoded_from_ids = tokenizer.decode(token_ids)
        
        assert decoded_from_tokens == test_sentence
        assert decoded_from_ids == test_sentence
        
        # Test persistence
        with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.json') as f:
            model_file = f.name
            
        try:
            tokenizer.save(model_file)
            
            # Load in new tokenizer
            new_tokenizer = unigram.Tokenizer.from_file(model_file)
            
            # Verify it works the same
            new_tokens = new_tokenizer.encode(test_sentence)
            assert new_tokens == tokens
        finally:
            if os.path.exists(model_file):
                os.unlink(model_file)
                
    def test_batch_encoding(self):
        """Test encoding multiple sentences"""
        config = unigram.TokenizerConfig()
        config.vocab_size = 200
        config.num_iterations = 2
        
        trainer = unigram.Trainer(config)
        corpus = "hello world test sentence " * 100
        vocab = trainer.train(corpus)
        
        tokenizer = unigram.Tokenizer(config)
        tokenizer.set_vocabulary(vocab)
        
        sentences = [
            "hello world",
            "test sentence",
            "hello test",
            "world sentence"
        ]
        
        # Encode all sentences
        all_tokens = [tokenizer.encode(s) for s in sentences]
        all_ids = [tokenizer.encode_as_ids(s) for s in sentences]
        
        # Verify each encoding
        for i, (tokens, ids, original) in enumerate(zip(all_tokens, all_ids, sentences)):
            assert len(tokens) > 0
            assert len(ids) > 0
            assert tokenizer.decode(tokens) == original
            assert tokenizer.decode(ids) == original
            
    def test_special_characters(self):
        """Test handling of special characters"""
        config = unigram.TokenizerConfig()
        config.vocab_size = 200
        config.num_iterations = 2
        config.byte_fallback = True
        
        trainer = unigram.Trainer(config)
        corpus = "hello world! How are you? I'm fine, thanks. " * 50
        vocab = trainer.train(corpus)
        
        tokenizer = unigram.Tokenizer(config)
        tokenizer.set_vocabulary(vocab)
        
        test_cases = [
            "Hello, world!",
            "How are you?",
            "I'm fine, thanks.",
            "Test: special-characters_here",
        ]
        
        for text in test_cases:
            tokens = tokenizer.encode(text)
            decoded = tokenizer.decode(tokens)
            assert decoded == text, f"Failed for: {text}"
            
    def test_empty_and_edge_cases(self):
        """Test edge cases"""
        config = unigram.TokenizerConfig()
        config.vocab_size = 100
        config.num_iterations = 2
        
        trainer = unigram.Trainer(config)
        corpus = "test " * 50
        vocab = trainer.train(corpus)
        
        tokenizer = unigram.Tokenizer(config)
        tokenizer.set_vocabulary(vocab)
        
        # Empty string
        tokens = tokenizer.encode("")
        assert isinstance(tokens, list)
        
        # Single character
        tokens = tokenizer.encode("a")
        assert len(tokens) > 0
        
        # Repeated characters
        tokens = tokenizer.encode("aaaa")
        assert len(tokens) > 0


class TestProgressCallback:
    def test_progress_callback(self):
        """Test training with progress callback"""
        config = unigram.TokenizerConfig()
        config.vocab_size = 100
        config.num_iterations = 3
        
        trainer = unigram.Trainer(config)
        
        progress_calls = []
        
        def callback(iteration, message):
            progress_calls.append((iteration, message))
            
        trainer.set_progress_callback(callback)
        
        corpus = "hello world " * 100
        vocab = trainer.train(corpus)
        
        # Verify callback was called
        assert len(progress_calls) > 0
        assert vocab.size() > 0


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
