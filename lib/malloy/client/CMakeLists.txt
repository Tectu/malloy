include(external.cmake)
include("../target_setup.cmake")

# Define target name
set(TARGET "malloy-client")

# Create library
malloy_add_library(${TARGET})

# Apply common setup
malloy_target_common_setup(${TARGET})

# Add subdirectories
add_subdirectory(http)
add_subdirectory(websocket)

# Add files
target_sources(
    ${TARGET}
    PRIVATE
        controller.cpp
        controller.hpp
        type_traits.hpp
)

target_include_directories(
    ${TARGET}
    PRIVATE
        3rdparty
)

target_link_libraries(
    ${TARGET}
    PUBLIC
        malloy-core
)
