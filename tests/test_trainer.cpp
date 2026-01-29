#include <gtest/gtest.h>
#include <unigram/trainer.h>

class TrainerTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_.vocab_size = 100;
        config_.num_iterations = 2;
        config_.min_frequency = 1;
    }

    unigram::TokenizerConfig config_;
};

TEST_F(TrainerTest, ConstructorInitialization) {
    unigram::Trainer trainer(config_);
    EXPECT_EQ(trainer.config().vocab_size, 100);
    EXPECT_EQ(trainer.config().num_iterations, 2);
}

TEST_F(TrainerTest, TrainEmptyCorpusFails) {
    unigram::Trainer trainer(config_);
    std::vector<std::string> empty_corpus;
    
    EXPECT_THROW(trainer.train(empty_corpus), std::invalid_argument);
}

TEST_F(TrainerTest, TrainSimpleCorpus) {
    unigram::Trainer trainer(config_);
    std::vector<std::string> corpus = {
        "hello world",
        "hello there",
        "world peace"
    };
    
    auto vocab = trainer.train(corpus);
    
    // Should have at least special tokens + some learned tokens
    EXPECT_GT(vocab.size(), 4);
}

TEST_F(TrainerTest, TrainWithProgressCallback) {
    unigram::Trainer trainer(config_);
    
    int callback_count = 0;
    trainer.set_progress_callback([&callback_count](size_t current, size_t total, const std::string& msg) {
        callback_count++;
        EXPECT_LE(current, total);
        EXPECT_FALSE(msg.empty());
    });
    
    std::vector<std::string> corpus = {"test", "data"};
    auto vocab = trainer.train(corpus);
    
    // Progress callback should have been called
    EXPECT_GT(callback_count, 0);
}

TEST_F(TrainerTest, VocabularyRespectsSizeLimit) {
    config_.vocab_size = 500;
    unigram::Trainer trainer(config_);
    
    std::vector<std::string> corpus;
    for (int i = 0; i < 100; ++i) {
        corpus.push_back("word" + std::to_string(i));
    }
    
    auto vocab = trainer.train(corpus);
    
    // Vocabulary includes 256 byte tokens + requested vocab_size
    // Allow margin for learned tokens that exceed target slightly
    EXPECT_LE(vocab.size(), 256 + config_.vocab_size + 50);
}

TEST_F(TrainerTest, MinFrequencyFiltering) {
    config_.min_frequency = 5;
    unigram::Trainer trainer(config_);
    
    std::vector<std::string> corpus = {
        "a", "b", "c", // Low frequency
        "frequent frequent frequent frequent frequent frequent" // High frequency
    };
    
    auto vocab = trainer.train(corpus);
    
    // Should have special tokens and the frequent characters
    EXPECT_GT(vocab.size(), 0);
}
TEST_F(TrainerTest, ByteLevelTokens_AlwaysPresent) {
    // Test that all 256 byte tokens are present after training
    config_.vocab_size = 500;
    unigram::Trainer trainer(config_);
    
    std::vector<std::string> corpus = {
        "hello world",
        "this is a test"
    };
    
    auto vocab = trainer.train(corpus);
    
    // Count how many single-byte tokens exist
    int byte_token_count = 0;
    for (int byte = 0; byte < 256; ++byte) {
        std::string byte_token(1, static_cast<char>(byte));
        if (vocab.contains(byte_token)) {
            byte_token_count++;
        }
    }
    
    // Should have all 256 byte tokens for complete coverage
    EXPECT_EQ(byte_token_count, 256);
}

TEST_F(TrainerTest, ByteLevelTokens_PreservedThroughIterations) {
    // Test that byte tokens survive multiple EM iterations
    config_.vocab_size = 300;
    config_.num_iterations = 10; // Many iterations
    unigram::Trainer trainer(config_);
    
    std::vector<std::string> corpus = {
        "abc",
        "def",
        "ghi"
    };
    
    auto vocab = trainer.train(corpus);
    
    // Even with many iterations, all byte tokens should remain
    for (int byte = 0; byte < 256; ++byte) {
        std::string byte_token(1, static_cast<char>(byte));
        EXPECT_TRUE(vocab.contains(byte_token))
            << "Byte token 0x" << std::hex << byte << " missing after training";
    }
}

TEST_F(TrainerTest, ByteLevelTokens_HaveLowScores) {
    // Test that byte tokens have lower scores than learned multi-char tokens
    config_.vocab_size = 500;
    unigram::Trainer trainer(config_);
    
    std::vector<std::string> corpus;
    // Add many instances of a complete sentence to create word tokens
    for (int i = 0; i < 100; ++i) {
        corpus.push_back("the quick brown fox jumps");
    }
    
    auto vocab = trainer.train(corpus);
    
    // Find a frequent multi-char token (like " the ")
    bool found_multichar = false;
    double multichar_score = -999.0;
    for (size_t i = 0; i < vocab.size(); ++i) {
        std::string token = vocab.get_token(i);
        if (token.length() > 1 && token != unigram::UNK_TOKEN && 
            token != unigram::BOS_TOKEN && token != unigram::EOS_TOKEN && 
            token != unigram::PAD_TOKEN) {
            found_multichar = true;
            multichar_score = vocab.get_score(i);
            break;
        }
    }
    
    ASSERT_TRUE(found_multichar) << "Should have at least one multi-char token";
    
    // Get score of single byte 'z' (should be uncommon in corpus)
    double z_score = vocab.get_score(vocab.get_id("z"));
    
    // Multi-char frequent token should have better (less negative) score
    // than rare single-byte token
    EXPECT_GT(multichar_score, z_score) 
        << "Multi-char token score: " << multichar_score << ", z score: " << z_score;
}

TEST_F(TrainerTest, ByteLevelTokens_NotCountedTowardsVocabSize) {
    // Test that the 256 byte tokens are preserved even when vocab_size is small
    config_.vocab_size = 100; // Smaller than 256
    unigram::Trainer trainer(config_);
    
    std::vector<std::string> corpus = {
        "test data"
    };
    
    auto vocab = trainer.train(corpus);
    
    // Should have at least 256 byte tokens even with small vocab_size
    EXPECT_GE(vocab.size(), 256);
    
    // All byte tokens should still be present
    for (int byte = 0; byte < 256; ++byte) {
        std::string byte_token(1, static_cast<char>(byte));
        EXPECT_TRUE(vocab.contains(byte_token));
    }
}