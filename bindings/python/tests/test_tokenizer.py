"""
Tests for the Unigram Tokenizer Python bindings
"""
import pytest
import tempfile
import os


try:
    from unigram_tokeniser import (
        Tokenizer, TokenizerConfig, Trainer, Vocabulary,
        UNK_TOKEN, BOS_TOKEN, EOS_TOKEN, PAD_TOKEN,
        UNK_TOKEN_ID, BOS_TOKEN_ID, EOS_TOKEN_ID, PAD_TOKEN_ID,
    )
    import unigram_tokeniser
    unigram = unigram_tokeniser.unigram  # For backward compatibility
except ImportError as e:
    pytest.skip(f"unigram_tokeniser module not built: {e}", allow_module_level=True)


class TestTokenizerConfig:
    def test_default_config(self):
        config = TokenizerConfig()
        assert config.vocab_size == 32000
        assert config.num_iterations > 0
        
    def test_config_modification(self):
        config = TokenizerConfig()
        config.vocab_size = 16000
        config.byte_fallback = True
        assert config.vocab_size == 16000
        assert config.byte_fallback


class TestVocabulary:
    def test_empty_vocabulary(self):
        vocab = Vocabulary()
        assert len(vocab) == 0
        
    def test_add_token(self):
        vocab = Vocabulary()
        token_id = vocab.add_token("hello", -1.5)
        assert vocab.size() > 0
        assert vocab.contains("hello")
        
    def test_get_token(self):
        vocab = Vocabulary()
        token_id = vocab.add_token("world", -2.0)
        assert vocab.get_token(token_id) == "world"
        
    def test_get_score(self):
        vocab = Vocabulary()
        score = -1.5
        token_id = vocab.add_token("test", score)
        assert abs(vocab.get_score(token_id) - score) < 0.001


class TestTrainer:
    def test_trainer_creation(self):
        config = TokenizerConfig()
        config.vocab_size = 1000
        trainer = Trainer(config)
        assert trainer.config().vocab_size == 1000
        
    def test_train_from_text(self):
        config = TokenizerConfig()
        config.vocab_size = 100
        config.num_iterations = 2
        
        trainer = Trainer(config)
        corpus = "hello world " * 100 + "test tokenizer " * 50
        
        vocab = trainer.train(corpus)
        assert vocab.size() > 0
        assert vocab.contains(" ")  # Space should be in vocabulary
        
    def test_train_from_file(self):
        config = TokenizerConfig()
        config.vocab_size = 100
        config.num_iterations = 2
        
        # Create temporary file with test data
        with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.txt') as f:
            f.write("hello world\n" * 100)
            f.write("test tokenizer\n" * 50)
            temp_file = f.name
            
        try:
            trainer = Trainer(config)
            vocab = trainer.train_from_file(temp_file)
            assert vocab.size() > 0
        finally:
            os.unlink(temp_file)


class TestTokenizer:
    def setup_method(self):
        """Setup a trained tokenizer for tests"""
        config = TokenizerConfig()
        config.vocab_size = 200
        config.num_iterations = 2
        
        trainer = Trainer(config)
        corpus = "hello world " * 100 + "test tokenizer " * 50 + "python bindings " * 30
        
        self.vocab = trainer.train(corpus)
        self.tokenizer = Tokenizer(config)
        self.tokenizer.set_vocabulary(self.vocab)
        
    def test_tokenizer_creation(self):
        config = TokenizerConfig()
        tokenizer = Tokenizer(config)
        assert tokenizer is not None
        
    def test_encode(self):
        tokens = self.tokenizer.encode("hello world")
        assert isinstance(tokens, list)
        assert len(tokens) > 0
        
    def test_encode_as_ids(self):
        token_ids = self.tokenizer.encode_as_ids("hello world")
        assert isinstance(token_ids, list)
        assert len(token_ids) > 0
        assert all(isinstance(tid, int) for tid in token_ids)
        
    def test_decode_tokens(self):
        text = "hello world"
        tokens = self.tokenizer.encode(text)
        decoded = self.tokenizer.decode(tokens)
        assert decoded == text
        
    def test_decode_ids(self):
        text = "hello world"
        token_ids = self.tokenizer.encode_as_ids(text)
        decoded = self.tokenizer.decode(token_ids)
        assert decoded == text
        
    def test_roundtrip(self):
        original_text = "test tokenizer python"
        tokens = self.tokenizer.encode(original_text)
        decoded = self.tokenizer.decode(tokens)
        assert decoded == original_text
        
    def test_save_and_load(self):
        # Create temporary file
        with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.json') as f:
            temp_file = f.name
            
        try:
            # Save tokenizer
            self.tokenizer.save(temp_file)
            assert os.path.exists(temp_file)
            
            # Load tokenizer
            config = TokenizerConfig()
            loaded_tokenizer = Tokenizer(config)
            loaded_tokenizer.load(temp_file)
            
            # Test that loaded tokenizer works
            text = "hello world"
            original_tokens = self.tokenizer.encode(text)
            loaded_tokens = loaded_tokenizer.encode(text)
            assert original_tokens == loaded_tokens
        finally:
            if os.path.exists(temp_file):
                os.unlink(temp_file)
                
    def test_from_file(self):
        # Create temporary file
        with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.json') as f:
            temp_file = f.name
            
        try:
            # Save tokenizer
            self.tokenizer.save(temp_file)
            
            # Load using from_file static method
            loaded_tokenizer = Tokenizer.from_file(temp_file)
            
            # Test that loaded tokenizer works
            text = "hello world"
            original_tokens = self.tokenizer.encode(text)
            loaded_tokens = loaded_tokenizer.encode(text)
            assert original_tokens == loaded_tokens
        finally:
            if os.path.exists(temp_file):
                os.unlink(temp_file)


class TestConstants:
    def test_special_tokens(self):
        assert UNK_TOKEN == "<unk>"
        assert BOS_TOKEN == "<s>"
        assert EOS_TOKEN == "</s>"
        assert PAD_TOKEN == "<pad>"
        
    def test_special_token_ids(self):
        assert isinstance(UNK_TOKEN_ID, int)
        assert isinstance(BOS_TOKEN_ID, int)
        assert isinstance(EOS_TOKEN_ID, int)
        assert isinstance(PAD_TOKEN_ID, int)
        assert UNK_TOKEN_ID == 0


class TestVersionInfo:
    def test_version_exists(self):
        assert hasattr(unigram_tokeniser, '__version__')
        assert isinstance(unigram_tokeniser.__version__, str)
        assert len(unigram_tokeniser.__version__) > 0


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
