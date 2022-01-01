cmake_minimum_required(VERSION 3.17)

# Project
project(
    malloy
    LANGUAGES CXX
    VERSION 0.2.0
)

# Options
option(MALLOY_BUILD_SHARED   "Whether to build as a shared library" OFF)
option(MALLOY_BUILD_EXAMPLES "Whether to build examples"       ON)
option(MALLOY_BUILD_TESTS    "Whether to build tests"          ON)
option(MALLOY_FEATURE_CLIENT "Whether to build the client"     ON)
option(MALLOY_FEATURE_SERVER "Whether to build the server"     ON)
option(MALLOY_FEATURE_HTML   "Whether to enable HTML features" ON)
option(MALLOY_FEATURE_TLS    "Whether to enable TLS features"  OFF)
option(MALLOY_FEATURE_APPFW  "Whether to enable the server-side application framework components" OFF)
option(MALLOY_DEPENDENCY_SPDLOG_DOWNLOAD "Whether to automatically fetch spdlog" ON)
option(MALLOY_DEPENDENCY_FMT_DOWNLOAD "Whether to automatically fetch fmt" ON)
option(MALLOY_DEPENDENCY_JSON_DOWNLOAD "Whether to automatically fetch nlohmann/json" ON)
option(MALLOY_DEPENDENCY_INJA_DOWNLOAD "Whether to automatically fetch pantor/inja" ON)

if (MALLOY_BUILD_TESTS)
    option(MALLOY_BUILD_CORO_TESTS "Whether to build the coroutine tests, currently disabled by default due to a gcc bug" OFF)
endif()
if (MALLOY_BUILD_EXAMPLES)
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        set(TMP ON)
    else()
        set(TMP OFF)
    endif()
    option(MALLOY_BUILD_CORO_EXAMPLES "Whether to build the coroutine examples, defaults to off for everything but gcc" ${TMP})
endif()

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

# Set dependencies accordingly
set(MALLOY_DEPENDENCY_OPENSSL OFF)
set(MALLOY_DEPENDENCY_JSON OFF)
set(MALLOY_DEPENDENCY_INJA OFF)
if (MALLOY_FEATURE_TLS)
    set(MALLOY_DEPENDENCY_OPENSSL ON)
endif()
if (MALLOY_FEATURE_APPFW)
    set(MALLOY_FEATURE_SERVER ON)
    set(MALLOY_FEATURE_HTML ON)
    set(MALLOY_DEPENDENCY_JSON ON)
    set(MALLOY_DEPENDENCY_INJA ON)
endif()

# Add the main library
add_subdirectory(lib)

# Add examples (if supposed to)
if (MALLOY_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Add tests (if supposed to)
if (MALLOY_BUILD_TESTS)
    add_subdirectory(test)
endif()

# Print options
message("")
message("-------------------------------")
message("Malloy configuration:")
message("")
message("  Build:")
message("    Examples      : " ${MALLOY_BUILD_EXAMPLES})
message("    Tests         : " ${MALLOY_BUILD_TESTS})
message("    Library Type  : " ${MALLOY_LIBRARY_TYPE})
message("")
message("  Features:")
message("    Client        : " ${MALLOY_FEATURE_CLIENT})
message("    Server        : " ${MALLOY_FEATURE_SERVER})
message("    HTML          : " ${MALLOY_FEATURE_HTML})
message("    TLS           : " ${MALLOY_FEATURE_TLS})
message("    App Framework : " ${MALLOY_FEATURE_APPFW})
message("")
message("  Dependencies:")
message("    OpenSSL       : " ${MALLOY_DEPENDENCY_OPENSSL})
message("    JSON          : " ${MALLOY_DEPENDENCY_JSON})
message("    inja          : " ${MALLOY_DEPENDENCY_INJA})
message("-------------------------------")
message("")

if (MALLOY_EXPORT_SYMBOLS)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ${MALLOY_TMP_WINEXPORT_ALL}) # Reset global var
endif()
