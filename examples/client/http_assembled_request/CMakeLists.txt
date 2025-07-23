# Set a target name
set(TARGET malloy-example-client-http-assembled-request)

# Create the executable
add_executable(${TARGET})

# Setup example
include(../example.cmake)
malloy_example_setup_client(${TARGET})

# Add source files
target_sources(
    ${TARGET}
    PRIVATE
        main.cpp
)
