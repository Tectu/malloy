include(external.cmake)
include("../target_setup.cmake")

# Define directory base path for FILE_SET
set(MALLOY_CLIENT_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Define target name
set(TARGET "malloy-client")

# Create library
add_library(${TARGET} ${MALLOY_LIBRARY_TYPE})

# Apply common setup
malloy_target_common_setup(${TARGET})

# Add subdirectories
add_subdirectory(http)
add_subdirectory(websocket)

# Add files
target_sources(
    ${TARGET}
    PUBLIC
        FILE_SET
            HEADERS
            BASE_DIRS ${MALLOY_CLIENT_BASE_DIR}
            FILES
                controller.hpp
                type_traits.hpp

    PRIVATE
        controller.cpp
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
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/malloy/client
)
