# Find OpenMP for parallel processing
find_package(OpenMP)

# Library target
add_library(unigram_tokeniser
    tokenizer.cpp
    vocabulary.cpp
    trainer.cpp
    internal/utf8_utils.cpp
)

# Alias for consistent naming
add_library(unigram::tokeniser ALIAS unigram_tokeniser)

# Set library properties
set_target_properties(unigram_tokeniser PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/unigram/tokenizer.h;${CMAKE_SOURCE_DIR}/include/unigram/vocabulary.h;${CMAKE_SOURCE_DIR}/include/unigram/trainer.h;${CMAKE_SOURCE_DIR}/include/unigram/types.h"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

# Include directories
target_include_directories(unigram_tokeniser
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries
if(OpenMP_CXX_FOUND)
    target_link_libraries(unigram_tokeniser PUBLIC OpenMP::OpenMP_CXX)
    target_compile_definitions(unigram_tokeniser PRIVATE USE_OPENMP)
endif()

target_link_libraries(unigram_tokeniser
    PRIVATE
        nlohmann_json::nlohmann_json
)

# Platform-specific compile definitions
if(BUILD_SHARED_LIBS)
    target_compile_definitions(unigram_tokeniser
        PRIVATE UNIGRAM_EXPORTS
        INTERFACE UNIGRAM_SHARED
    )
endif()

# Link libraries (add dependencies here as needed)
# Example: target_link_libraries(unigram_tokeniser PRIVATE some_lib)

# Installation
install(TARGETS unigram_tokeniser
    EXPORT UnigramTokeniserTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/unigram
)

# Export targets
install(EXPORT UnigramTokeniserTargets
    FILE UnigramTokeniserTargets.cmake
    NAMESPACE unigram::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/UnigramTokeniser
)
