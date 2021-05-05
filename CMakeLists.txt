cmake_minimum_required(VERSION 3.17)

# Project
project(
    malloy
    LANGUAGES CXX
    VERSION 0.0.0
)

# Options
option(BUILD_EXAMPLES "Whether to build examples"     ON)
option(BUILD_TESTS    "Whether to build tests"        ON)

# Add the main library
add_subdirectory(lib)

# Add examples (if supposed to)
if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Add tests (if supposed to)
if (BUILD_TESTS)
    add_subdirectory(test)
endif()

# Print options
message("")
message("---------------------")
message("Malloy configuration:")
message("  Build:")
message("    Examples : " ${BUILD_EXAMPLES})
message("    Tests    : " ${BUILD_TESTS})
message("---------------------")
message("")