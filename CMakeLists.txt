cmake_minimum_required(VERSION 3.23)

# Project
project(
    malloy
    VERSION 0.10.0
    LANGUAGES
        CXX
    HOMEPAGE_URL "https://github.com/tectu/malloy"
)

# Policies
if (POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

# Options
option(MALLOY_BUILD_SHARED   "Whether to build as a shared library" OFF)
option(MALLOY_BUILD_EXAMPLES "Whether to build examples"       ON)
option(MALLOY_BUILD_TESTS    "Whether to build tests"          ON)
option(MALLOY_FEATURE_CLIENT "Whether to build the client"     ON)
option(MALLOY_FEATURE_SERVER "Whether to build the server"     ON)
option(MALLOY_FEATURE_HTML   "Whether to enable HTML features" ON)
option(MALLOY_FEATURE_TLS    "Whether to enable TLS features"  OFF)
option(MALLOY_DEPENDENCY_SPDLOG_DOWNLOAD "Whether to automatically fetch spdlog" ON)
option(MALLOY_DEPENDENCY_FMT_DOWNLOAD "Whether to automatically fetch fmt" ON)

# Settings
set(MALLOY_WIN32_WINNT "0x0A00")

set(MALLOY_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin) # Needed for later

if (MSVC AND MALLOY_BUILD_SHARED)
    set(MALLOY_EXPORT_SYMBOLS ON)
else()
    set(MALLOY_EXPORT_SYMBOLS OFF)
endif()

if (MALLOY_EXPORT_SYMBOLS)
    set(MALLOY_TMP_WINEXPORT_ALL ${CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS})
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON) # See: https://cmake.org/cmake/help/latest/prop_tgt/WINDOWS_EXPORT_ALL_SYMBOLS.html
endif()

# Set library type to build
if (MALLOY_BUILD_SHARED) 
    set(MALLOY_LIBRARY_TYPE SHARED)
else()
    set(MALLOY_LIBRARY_TYPE STATIC)
endif()

# Disable server specific features if server feature is disabled
if (NOT MALLOY_FEATURE_SERVER)
    set(MALLOY_FEATURE_HTML OFF)
endif()

# Set dependencies accordingly
set(MALLOY_DEPENDENCY_OPENSSL OFF)
if (MALLOY_FEATURE_TLS)
    set(MALLOY_DEPENDENCY_OPENSSL ON)
endif()

# Dependency minimum versions
set(MALLOY_DEPENDENCY_BOOST_VERSION_MIN 1.86.0)

# Add the main library
add_subdirectory(lib)

# Add examples (if supposed to)
if (MALLOY_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Add tests (if supposed to)
if (MALLOY_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

# Print options
message(STATUS "")
message(STATUS "-------------------------------")
message(STATUS "Malloy configuration:")
message(STATUS "")
message(STATUS "  Build:")
message(STATUS "    Examples      : " ${MALLOY_BUILD_EXAMPLES})
message(STATUS "    Tests         : " ${MALLOY_BUILD_TESTS})
message(STATUS "    Library Type  : " ${MALLOY_LIBRARY_TYPE})
message(STATUS "")
message(STATUS "  Features:")
message(STATUS "    Client        : " ${MALLOY_FEATURE_CLIENT})
message(STATUS "    Server        : " ${MALLOY_FEATURE_SERVER})
message(STATUS "    HTML          : " ${MALLOY_FEATURE_HTML})
message(STATUS "    TLS           : " ${MALLOY_FEATURE_TLS})
message(STATUS "")
message(STATUS "  Dependencies:")
message(STATUS "    OpenSSL       : " ${MALLOY_DEPENDENCY_OPENSSL})
message(STATUS "-------------------------------")
message(STATUS "")

if (MALLOY_EXPORT_SYMBOLS)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ${MALLOY_TMP_WINEXPORT_ALL}) # Reset global var
endif()


########################################################################################################################
# CPack                                                                                                                #
########################################################################################################################
set(CPACK_PACKAGE_VENDOR "Joel Bodenmann")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A C++ library providing embeddable server & client components for both HTTP and WebSocket.")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/license.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
include(CPack)
