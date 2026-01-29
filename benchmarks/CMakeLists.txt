cmake_minimum_required(VERSION 3.20)

# Fetch Google Benchmark
include(FetchContent)
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
    GIT_SHALLOW TRUE
)

# Configure benchmark options
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(benchmark)

# Tokenizer benchmarks
add_executable(tokenizer_benchmark
    tokenizer_benchmark.cpp
)

target_link_libraries(tokenizer_benchmark PRIVATE
    unigram::tokeniser
    benchmark::benchmark
)

# Trainer benchmarks
add_executable(trainer_benchmark
    trainer_benchmark.cpp
)

target_link_libraries(trainer_benchmark PRIVATE
    unigram::tokeniser
    benchmark::benchmark
)

# Add benchmarks to CTest (optional - for integration with test runner)
add_test(NAME TokenizerBenchmark COMMAND tokenizer_benchmark --benchmark_min_time=0.1)
add_test(NAME TrainerBenchmark COMMAND trainer_benchmark --benchmark_min_time=0.1)
