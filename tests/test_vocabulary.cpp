#include <gtest/gtest.h>
#include <unigram/vocabulary.h>
#include <filesystem>
#include <fstream>

class VocabularyTest : public ::testing::Test {
protected:
    void SetUp() override {
        vocab_ = std::make_unique<unigram::Vocabulary>();
    }

    std::unique_ptr<unigram::Vocabulary> vocab_;
};

TEST_F(VocabularyTest, InitialSize) {
    // Should have special tokens
    EXPECT_GE(vocab_->size(), 4); // UNK, BOS, EOS, PAD
}

TEST_F(VocabularyTest, AddToken) {
    size_t initial_size = vocab_->size();
    auto id = vocab_->add_token("hello", 1.0);
    
    EXPECT_EQ(vocab_->size(), initial_size + 1);
    EXPECT_EQ(vocab_->get_token(id), "hello");
}

TEST_F(VocabularyTest, GetTokenById) {
    auto id = vocab_->add_token("world", 2.0);
    EXPECT_EQ(vocab_->get_token(id), "world");
}

TEST_F(VocabularyTest, GetIdByToken) {
    auto id = vocab_->add_token("test", 3.0);
    EXPECT_EQ(vocab_->get_id("test"), id);
}

TEST_F(VocabularyTest, GetUnknownToken) {
    auto id = vocab_->get_id("nonexistent");
    EXPECT_EQ(id, unigram::UNK_TOKEN_ID);
}

TEST_F(VocabularyTest, GetScore) {
    auto id = vocab_->add_token("scored", 5.5);
    EXPECT_DOUBLE_EQ(vocab_->get_score(id), 5.5);
}

TEST_F(VocabularyTest, Contains) {
    vocab_->add_token("exists", 1.0);
    
    EXPECT_TRUE(vocab_->contains("exists"));
    EXPECT_FALSE(vocab_->contains("doesnotexist"));
}

TEST_F(VocabularyTest, UpdateExistingToken) {
    auto id1 = vocab_->add_token("update", 1.0);
    auto id2 = vocab_->add_token("update", 2.0);
    
    // Should return same ID
    EXPECT_EQ(id1, id2);
    
    // Score should be updated
    EXPECT_DOUBLE_EQ(vocab_->get_score(id1), 2.0);
}

TEST_F(VocabularyTest, Clear) {
    vocab_->add_token("test1", 1.0);
    vocab_->add_token("test2", 2.0);
    
    vocab_->clear();
    
    // Should only have special tokens after clear
    EXPECT_EQ(vocab_->size(), 4);
    EXPECT_FALSE(vocab_->contains("test1"));
    EXPECT_FALSE(vocab_->contains("test2"));
}

TEST_F(VocabularyTest, SpecialTokens) {
    EXPECT_TRUE(vocab_->contains(unigram::UNK_TOKEN));
    EXPECT_TRUE(vocab_->contains(unigram::BOS_TOKEN));
    EXPECT_TRUE(vocab_->contains(unigram::EOS_TOKEN));
    EXPECT_TRUE(vocab_->contains(unigram::PAD_TOKEN));
}

TEST_F(VocabularyTest, SaveAndLoad) {
    // Add some tokens
    vocab_->add_token("hello", -1.5);
    vocab_->add_token("world", -2.0);
    vocab_->add_token("test", -1.8);
    
    size_t original_size = vocab_->size();
    
    // Save to file
    std::string temp_file = "test_vocab.json";
    ASSERT_TRUE(vocab_->save(temp_file));
    
    // Load into new vocabulary
    unigram::Vocabulary loaded_vocab;
    ASSERT_TRUE(loaded_vocab.load(temp_file));
    
    // Verify loaded vocabulary
    EXPECT_EQ(loaded_vocab.size(), original_size);
    EXPECT_TRUE(loaded_vocab.contains("hello"));
    EXPECT_TRUE(loaded_vocab.contains("world"));
    EXPECT_TRUE(loaded_vocab.contains("test"));
    
    EXPECT_DOUBLE_EQ(loaded_vocab.get_score(loaded_vocab.get_id("hello")), -1.5);
    EXPECT_DOUBLE_EQ(loaded_vocab.get_score(loaded_vocab.get_id("world")), -2.0);
    EXPECT_DOUBLE_EQ(loaded_vocab.get_score(loaded_vocab.get_id("test")), -1.8);
    
    // Cleanup
    std::filesystem::remove(temp_file);
}

TEST_F(VocabularyTest, LoadNonexistentFile) {
    unigram::Vocabulary vocab;
    EXPECT_FALSE(vocab.load("nonexistent_file.json"));
}

TEST_F(VocabularyTest, SaveInvalidPath) {
    EXPECT_FALSE(vocab_->save("/invalid/path/model.json"));
}
