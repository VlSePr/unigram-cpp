# CLI executable
add_executable(unigram-cli
    main.cpp
    commands/train.cpp
    commands/tokenize.cpp
    commands/encode.cpp
    commands/benchmark.cpp
    commands/compare.cpp
    utils/args_parser.cpp
)

# Link against the library
target_link_libraries(unigram-cli PRIVATE unigram::tokeniser)

# Include directories
target_include_directories(unigram-cli PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Installation
install(TARGETS unigram-cli
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
