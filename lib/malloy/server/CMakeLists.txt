include(external.cmake)
include(../target_setup.cmake)

# Define target name
set(TARGET "malloy-server")

# Create library
add_library(${TARGET} ${MALLOY_LIBRARY_TYPE})


# Apply common setup
malloy_target_common_setup(${TARGET})

# Add subdirectories
add_subdirectory(http)
add_subdirectory(routing)
add_subdirectory(websocket)

# Add sources
target_sources(
    ${TARGET}
    PUBLIC
        application.hpp
    PRIVATE
        application.cpp
        controller.cpp
        controller.hpp
        listener.cpp
        listener.hpp
)

# Link libraries
target_link_libraries(
    ${TARGET}
    PUBLIC
        malloy-core
)
