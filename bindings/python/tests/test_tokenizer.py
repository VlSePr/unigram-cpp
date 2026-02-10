"""
Tests for the Unigram Tokenizer Python bindings
"""
import pytest
import tempfile
import os


try:
    import unigram
except ImportError:
    pytest.skip("unigram module not built", allow_module_level=True)


class TestTokenizerConfig:
    def test_default_config(self):
        config = unigram.TokenizerConfig()
        assert config.vocab_size == 32000
        assert config.num_iterations > 0
        
    def test_config_modification(self):
        config = unigram.TokenizerConfig()
        config.vocab_size = 16000
        config.byte_fallback = True
        assert config.vocab_size == 16000
        assert config.byte_fallback


class TestVocabulary:
    def test_empty_vocabulary(self):
        vocab = unigram.Vocabulary()
        assert len(vocab) == 0
        
    def test_add_token(self):
        vocab = unigram.Vocabulary()
        token_id = vocab.add_token("hello", -1.5)
        assert vocab.size() > 0
        assert vocab.contains("hello")
        
    def test_get_token(self):
        vocab = unigram.Vocabulary()
        token_id = vocab.add_token("world", -2.0)
        assert vocab.get_token(token_id) == "world"
        
    def test_get_score(self):
        vocab = unigram.Vocabulary()
        score = -1.5
        token_id = vocab.add_token("test", score)
        assert abs(vocab.get_score(token_id) - score) < 0.001


class TestTrainer:
    def test_trainer_creation(self):
        config = unigram.TokenizerConfig()
        config.vocab_size = 1000
        trainer = unigram.Trainer(config)
        assert trainer.config().vocab_size == 1000
        
    def test_train_from_text(self):
        config = unigram.TokenizerConfig()
        config.vocab_size = 100
        config.num_iterations = 2
        
        trainer = unigram.Trainer(config)
        corpus = "hello world " * 100 + "test tokenizer " * 50
        
        vocab = trainer.train(corpus)
        assert vocab.size() > 0
        assert vocab.contains(" ")  # Space should be in vocabulary
        
    def test_train_from_file(self):
        config = unigram.TokenizerConfig()
        config.vocab_size = 100
        config.num_iterations = 2
        
        # Create temporary file with test data
        with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.txt') as f:
            f.write("hello world\n" * 100)
            f.write("test tokenizer\n" * 50)
            temp_file = f.name
            
        try:
            trainer = unigram.Trainer(config)
            vocab = trainer.train_from_file(temp_file)
            assert vocab.size() > 0
        finally:
            os.unlink(temp_file)


class TestTokenizer:
    def setup_method(self):
        """Setup a trained tokenizer for tests"""
        config = unigram.TokenizerConfig()
        config.vocab_size = 200
        config.num_iterations = 2
        
        trainer = unigram.Trainer(config)
        corpus = "hello world " * 100 + "test tokenizer " * 50 + "python bindings " * 30
        
        self.vocab = trainer.train(corpus)
        self.tokenizer = unigram.Tokenizer(config)
        self.tokenizer.set_vocabulary(self.vocab)
        
    def test_tokenizer_creation(self):
        config = unigram.TokenizerConfig()
        tokenizer = unigram.Tokenizer(config)
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
            config = unigram.TokenizerConfig()
            loaded_tokenizer = unigram.Tokenizer(config)
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
            loaded_tokenizer = unigram.Tokenizer.from_file(temp_file)
            
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
        assert unigram.UNK_TOKEN == "<unk>"
        assert unigram.BOS_TOKEN == "<s>"
        assert unigram.EOS_TOKEN == "</s>"
        assert unigram.PAD_TOKEN == "<pad>"
        
    def test_special_token_ids(self):
        assert isinstance(unigram.UNK_TOKEN_ID, int)
        assert isinstance(unigram.BOS_TOKEN_ID, int)
        assert isinstance(unigram.EOS_TOKEN_ID, int)
        assert isinstance(unigram.PAD_TOKEN_ID, int)
        assert unigram.UNK_TOKEN_ID == 0


class TestVersionInfo:
    def test_version_exists(self):
        assert hasattr(unigram, '__version__')
        assert isinstance(unigram.__version__, str)
        assert len(unigram.__version__) > 0


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
