# Set a target name
set(TARGET malloy-example-server-applications-demo01)

# Create the executable
add_executable(${TARGET})

# Setup example
include(../../example.cmake)
malloy_example_setup(${TARGET})

add_subdirectory(3rdparty/sqlite)
add_subdirectory(apps)

target_include_directories(
    ${TARGET}
    PRIVATE
        3rdparty/sqlite
)

target_sources(
    ${TARGET}
    PRIVATE
        main.cpp
        database.cpp
        database.hpp
)

target_link_libraries(
    ${TARGET}
    PRIVATE
        sqlite3
)
