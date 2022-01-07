include(external.cmake)
include(../target_setup.cmake)

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
if (MALLOY_FEATURE_TLS)
    add_subdirectory(tls)
endif()
add_subdirectory(websocket)
add_subdirectory(detail)

# Add files
target_sources(
    ${TARGET}
    PUBLIC
        controller.hpp
        error.hpp
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
