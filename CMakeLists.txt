cmake_minimum_required(VERSION 3.17)

# Project
project(
    malloy
    LANGUAGES CXX
    VERSION 0.0.0
)

# Options
option(BUILD_EXAMPLES "Whether to build examples" OFF)

# Add the main library
add_subdirectory(lib)

# Add examples (if supposed to)
if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
