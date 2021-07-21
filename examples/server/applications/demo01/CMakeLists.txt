# Set a target name
set(TARGET malloy-example-server-applications-demo01)

# Create the executable
add_executable(${TARGET})

# Setup example
include(../../example.cmake)
malloy_example_setup(${TARGET})

add_subdirectory(apps)

target_sources(
    ${TARGET}
    PRIVATE
        main.cpp
)
