include(CMakeFindDependencyMacro)

# Capture dependency configurations
set(MALLOY_DEPENDENCY_OPENSSL OFF)

# Include dependencies
find_dependency(fmt REQUIRED)
find_dependency(spdlog REQUIRED)
find_dependency(
    Boost
    1.86.0
    REQUIRED
    COMPONENTS
        url
)
if (MALLOY_DEPENDENCY_OPENSSL)
    find_dependency(OpenSSL REQUIRED)
endif()

# Add the targets file
include("${CMAKE_CURRENT_LIST_DIR}/malloy-targets.cmake")
