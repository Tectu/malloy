include(external.cmake)
include(../target_setup.cmake)

# Define directory base path for FILE_SET
set(MALLOY_SERVER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Define target name
set(TARGET "malloy-server")

# Create library
add_library(${TARGET} ${MALLOY_LIBRARY_TYPE})

# Apply common setup
malloy_target_common_setup(${TARGET})

# Add subdirectories
add_subdirectory(http)
add_subdirectory(rest)
add_subdirectory(routing)
add_subdirectory(websocket)
add_subdirectory(auth)

# Add sources
target_sources(
    ${TARGET}
    PUBLIC
        FILE_SET
            HEADERS
            BASE_DIRS ${MALLOY_SERVER_BASE_DIR}
            FILES
                routing_context.hpp
                listener.hpp

    PRIVATE
        routing_context.cpp
        listener.cpp
)

# Link libraries
target_link_libraries(
    ${TARGET}
    PUBLIC
        malloy-core
)



########################################################################################################################
# Install                                                                                                              #
########################################################################################################################

include(GNUInstallDirs)

install(
    TARGETS
        ${TARGET}
    EXPORT malloy-targets
    FILE_SET
        HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/malloy/server
)
