# Fetch Google Test
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Test executable
add_executable(unigram_tests
    test_tokenizer.cpp
    test_vocabulary.cpp
    test_trainer.cpp
)

# Link against library and Google Test
target_link_libraries(unigram_tests PRIVATE
    unigram::tokeniser
    GTest::gtest_main
)

# Include Google Test
include(GoogleTest)
gtest_discover_tests(unigram_tests)
