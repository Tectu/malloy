# Set a target name
set(TARGET malloy-example-server-applications-demo01)

# Create the executable
add_executable(${TARGET})

# Setup example
include(../../example.cmake)
malloy_example_setup(${TARGET})

add_subdirectory(3rdparty/sqlite)
add_subdirectory(apps)

target_sources(
    ${TARGET}
    PRIVATE
        main.cpp
        app.cpp
        app.hpp
        form_renderer.cpp
        form_renderer.hpp
        database.cpp
        database.hpp
        page_master.hpp
)
