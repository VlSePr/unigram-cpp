# Python bindings using pybind11
# To enable: cmake .. -DBUILD_PYTHON_BINDINGS=ON

find_package(Python COMPONENTS Interpreter Development REQUIRED)

include(FetchContent)

FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v2.11.1
)

FetchContent_MakeAvailable(pybind11)

pybind11_add_module(unigram_py
    bindings.cpp
)

target_link_libraries(unigram_py PRIVATE
    unigram::tokeniser
)

# Set output name to just 'unigram' for Python import
set_target_properties(unigram_py PROPERTIES
    OUTPUT_NAME unigram
)

# Installation for Python package
install(TARGETS unigram_py
    LIBRARY DESTINATION ${Python_SITEARCH}
)
