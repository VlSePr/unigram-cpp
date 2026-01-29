# Java bindings using JNI
# To enable: cmake .. -DBUILD_JAVA_BINDINGS=ON

find_package(JNI REQUIRED)

# Create JNI wrapper library
add_library(unigram_jni SHARED
    jni/com_unigram_Tokenizer.cpp
    jni/com_unigram_Vocabulary.cpp
)

target_include_directories(unigram_jni PRIVATE
    ${JNI_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/jni
)

target_link_libraries(unigram_jni PRIVATE
    unigram::tokeniser
)

# Installation
install(TARGETS unigram_jni
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Note: Java class files need to be built separately with javac
# The CMake build only handles the native JNI library
