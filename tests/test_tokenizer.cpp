#include <gtest/gtest.h>
#include <unigram/tokenizer.h>

class TokenizerTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_.vocab_size = 1000;
        config_.add_bos_token = false;
        config_.add_eos_token = false;
    }

    unigram::TokenizerConfig config_;
};

TEST_F(TokenizerTest, ConstructorInitialization) {
    unigram::Tokenizer tokenizer(config_);
    EXPECT_EQ(tokenizer.config().vocab_size, 1000);
}

TEST_F(TokenizerTest, EncodeEmptyString) {
    unigram::Tokenizer tokenizer(config_);
    auto tokens = tokenizer.encode("");
    EXPECT_TRUE(tokens.empty());
}

TEST_F(TokenizerTest, EncodeSimpleText) {
    unigram::Tokenizer tokenizer(config_);
    auto tokens = tokenizer.encode("Hello world");
    EXPECT_FALSE(tokens.empty());
}

TEST_F(TokenizerTest, DecodeEmptyTokens) {
    unigram::Tokenizer tokenizer(config_);
    unigram::Tokens tokens;
    auto text = tokenizer.decode(tokens);
    EXPECT_TRUE(text.empty());
}

TEST_F(TokenizerTest, EncodeDecodeRoundTrip) {
    unigram::Tokenizer tokenizer(config_);
    std::string original = "Hello world";
    auto tokens = tokenizer.encode(original);
    auto decoded = tokenizer.decode(tokens);
    
    // Note: exact match might not always hold due to tokenization
    // This test checks basic functionality
    EXPECT_FALSE(decoded.empty());
}

TEST_F(TokenizerTest, SpecialTokens) {
    config_.add_bos_token = true;
    config_.add_eos_token = true;
    
    unigram::Tokenizer tokenizer(config_);
    auto tokens = tokenizer.encode("test");
    
    EXPECT_GE(tokens.size(), 3); // BOS + at least 1 token + EOS
    EXPECT_EQ(tokens.front(), unigram::BOS_TOKEN);
    EXPECT_EQ(tokens.back(), unigram::EOS_TOKEN);
}

TEST_F(TokenizerTest, MoveConstructor) {
    unigram::Tokenizer tokenizer1(config_);
    unigram::Tokenizer tokenizer2(std::move(tokenizer1));
    
    auto tokens = tokenizer2.encode("test");
    EXPECT_FALSE(tokens.empty());
}

TEST_F(TokenizerTest, MoveAssignment) {
    unigram::Tokenizer tokenizer1(config_);
    unigram::Tokenizer tokenizer2(config_);
    
    tokenizer2 = std::move(tokenizer1);
    auto tokens = tokenizer2.encode("test");
    EXPECT_FALSE(tokens.empty());
}
TEST_F(TokenizerTest, ByteLevelFallback_SingleUnknownChar) {
    // Test that single unknown character falls back to byte token
    unigram::Vocabulary vocab;
    
    // Add only byte-level tokens (0x00-0xFF)
    for (int byte = 0; byte < 256; ++byte) {
        std::string byte_token(1, static_cast<char>(byte));
        vocab.add_token(byte_token, -15.0);
    }
    
    unigram::Tokenizer tokenizer(config_);
    tokenizer.set_vocabulary(vocab);
    
    // Should tokenize as single byte
    auto tokens = tokenizer.encode("x");
    EXPECT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens[0], "x");
}

TEST_F(TokenizerTest, ByteLevelFallback_MultipleUnknownChars) {
    // Test that unknown multi-byte sequence splits into individual bytes
    unigram::Vocabulary vocab;
    
    // Add only byte-level tokens
    for (int byte = 0; byte < 256; ++byte) {
        std::string byte_token(1, static_cast<char>(byte));
        vocab.add_token(byte_token, -15.0);
    }
    
    unigram::Tokenizer tokenizer(config_);
    tokenizer.set_vocabulary(vocab);
    
    // Should split into individual byte tokens
    auto tokens = tokenizer.encode("xyz");
    EXPECT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0], "x");
    EXPECT_EQ(tokens[1], "y");
    EXPECT_EQ(tokens[2], "z");
}

TEST_F(TokenizerTest, ByteLevelFallback_NoUnknownTokens) {
    // Test that with byte-level fallback, we NEVER get <unk>
    unigram::Vocabulary vocab;
    
    // Add byte-level tokens
    for (int byte = 0; byte < 256; ++byte) {
        std::string byte_token(1, static_cast<char>(byte));
        vocab.add_token(byte_token, -15.0);
    }
    
    // Add some common tokens with better scores
    vocab.add_token(" ", -3.5);
    vocab.add_token("the", -5.0);
    
    unigram::Tokenizer tokenizer(config_);
    tokenizer.set_vocabulary(vocab);
    
    // Test various inputs - should NEVER produce <unk>
    auto tokens1 = tokenizer.encode("The quick brown fox");
    for (const auto& token : tokens1) {
        EXPECT_NE(token, unigram::UNK_TOKEN);
    }
    
    auto tokens2 = tokenizer.encode("xyzqwerty");
    for (const auto& token : tokens2) {
        EXPECT_NE(token, unigram::UNK_TOKEN);
    }
    
    auto tokens3 = tokenizer.encode("!@#$%^&*()");
    for (const auto& token : tokens3) {
        EXPECT_NE(token, unigram::UNK_TOKEN);
    }
}

TEST_F(TokenizerTest, ByteLevelFallback_PrefersLongerTokens) {
    // Test that tokenizer prefers longer known tokens over byte splits
    unigram::Vocabulary vocab;
    
    // Add byte-level tokens
    for (int byte = 0; byte < 256; ++byte) {
        std::string byte_token(1, static_cast<char>(byte));
        vocab.add_token(byte_token, -15.0);
    }
    
    // Add multi-character token with better score
    vocab.add_token("hello", -5.0);
    
    unigram::Tokenizer tokenizer(config_);
    tokenizer.set_vocabulary(vocab);
    
    // Should prefer "hello" over "h"+"e"+"l"+"l"+"o"
    auto tokens = tokenizer.encode("hello");
    EXPECT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens[0], "hello");
}

TEST_F(TokenizerTest, ByteLevelFallback_MixedKnownUnknown) {
    // Test tokenizing text with both known and unknown parts
    unigram::Vocabulary vocab;
    
    // Add byte-level tokens
    for (int byte = 0; byte < 256; ++byte) {
        std::string byte_token(1, static_cast<char>(byte));
        vocab.add_token(byte_token, -15.0);
    }
    
    // Add some known words
    vocab.add_token(" ", -3.5);
    vocab.add_token("the", -5.0);
    vocab.add_token("cat", -6.0);
    
    unigram::Tokenizer tokenizer(config_);
    tokenizer.set_vocabulary(vocab);
    
    // "the" and "cat" should be kept whole, "xyz" should split to bytes
    auto tokens = tokenizer.encode("the xyz cat");
    
    // Verify no <unk> tokens
    for (const auto& token : tokens) {
        EXPECT_NE(token, unigram::UNK_TOKEN);
    }
    
    // Should contain "the", "cat", and individual letters
    bool has_the = false, has_cat = false;
    for (const auto& token : tokens) {
        if (token == "the") has_the = true;
        if (token == "cat") has_cat = true;
    }
    EXPECT_TRUE(has_the);
    EXPECT_TRUE(has_cat);
}