include(external.cmake)
include(../target_setup.cmake)

# Define directory base path for FILE_SET
set(MALLOY_CORE_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Define target name
set(TARGET "malloy-core")

# Create library
add_library(${TARGET} ${MALLOY_LIBRARY_TYPE})

# Apply common setup
malloy_target_common_setup(${TARGET})

# Add subdirectories
if (MALLOY_FEATURE_HTML)
    add_subdirectory(html)
endif()
add_subdirectory(http)
add_subdirectory(tcp)
if (MALLOY_FEATURE_TLS)
    add_subdirectory(tls)
endif()
add_subdirectory(websocket)
add_subdirectory(detail)

# Add files
target_sources(
    ${TARGET}
    PUBLIC
        FILE_SET
            HEADERS
            BASE_DIRS ${MALLOY_CORE_BASE_DIR}
            FILES
                controller.hpp
                error.hpp
                mp.hpp
                type_traits.hpp
                utils.hpp
)

# Set _WIN32_WINNT correctly (if needed)
if (WIN32)
    include (../win32_version.cmake)

    get_WIN32_WINNT(winnt_version)

    target_compile_definitions(
        ${TARGET}
        PUBLIC
            _WIN32_WINNT=${winnt_version}   # Keep this public so user's can't accidentally use conflicting versions
    )
endif()



########################################################################################################################
# Install                                                                                                              #
########################################################################################################################

include(GNUInstallDirs)

# Targets
install(
    TARGETS
        ${TARGET}
    EXPORT malloy-targets
    FILE_SET
        HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/malloy/core
)
if (MALLOY_DEPENDENCY_FMT_DOWNLOAD)
    install(
        TARGETS
            fmt
        EXPORT malloy-targets
    )
endif()
if (MALLOY_DEPENDENCY_SPDLOG_DOWNLOAD)
    install(
        TARGETS
            spdlog
        EXPORT malloy-targets
    )
endif()
