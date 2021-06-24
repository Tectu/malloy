# Set a target name
set(TARGET malloy-example-server-routing-regex)

# Create the executable
add_executable(${TARGET})

# Setup example
include(../../example.cmake)
malloy_example_setup(${TARGET})

# Add source files
target_sources(
    ${TARGET}
    PRIVATE
        main.cpp
)
