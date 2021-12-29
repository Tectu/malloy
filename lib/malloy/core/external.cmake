include(FetchContent)

set(MALLOY_TMP_BINDIR ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MALLOY_BINARY_DIR})


########################################################################################################################
# Boost
########################################################################################################################
find_package(
    Boost
    1.74.0
    REQUIRED
)


########################################################################################################################
# fmt
########################################################################################################################
if (MALLOY_DEPENDENCY_FMT_DOWNLOAD)
    FetchContent_Declare(
        fmt 
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG 8.0.1 # Supported by spdlog 1.9.0
    )
    FetchContent_GetProperties(fmt)
    if (NOT fmt_POPULATED) 
        FetchContent_Populate(fmt)
        set(MALLOY_TMP_VAR1 ${BUILD_SHARED_LIBS})
        set(BUILD_SHARED_LIBS ${MALLOY_BUILD_SHARED}) # fmt has no specific shared option

        add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR})

        set(BUILD_SHARED_LIBS ${MALLOY_TMP_VAR1})

    endif()
else()
    find_package(fmt REQUIRED)
endif()


########################################################################################################################
# spdlog
########################################################################################################################
if (MALLOY_DEPENDENCY_SPDLOG_DOWNLOAD)
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog
        GIT_TAG        v1.9.2
    )
    FetchContent_GetProperties(spdlog)
    if (NOT spdlog_POPULATED) 
        FetchContent_Populate(spdlog)
        set(SPDLOG_BUILD_SHARED ${MALLOY_BUILD_SHARED} CACHE INTERNAL "")
        set(SPDLOG_FMT_EXTERNAL ON CACHE INTERNAL "")

        add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR})
    endif()
else()
    find_package(spdlog REQUIRED)
endif()


########################################################################################################################
# OpenSSL
########################################################################################################################
if (MALLOY_DEPENDENCY_OPENSSL)
    find_package(
        OpenSSL
        REQUIRED
    )
endif()


########################################################################################################################
# nlohmann/json: https://github.com/nlohmann/json
########################################################################################################################
if (MALLOY_DEPENDENCY_JSON_DOWNLOAD)
    FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.9.1
    )
    FetchContent_GetProperties(json)
    if (NOT json_POPULATED)
        FetchContent_Populate(json)

        set(JSON_BuildTests OFF CACHE INTERNAL "")
        add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR})
    endif()
endif()


########################################################################################################################
# pantor/inja: https://github.com/pantor/inja
########################################################################################################################
if (MALLOY_DEPENDENCY_INJA)
    FetchContent_Declare(
        inja
        GIT_REPOSITORY https://github.com/pantor/inja.git
        GIT_TAG        v3.3.0
    )
    FetchContent_GetProperties(inja)
    if(NOT inja_POPULATED)
        FetchContent_Populate(inja)
        set(INJA_USE_EMBEDDED_JSON OFF CACHE INTERNAL "")
        set(INJA_INSTALL           OFF CACHE INTERNAL "")
        set(INJA_EXPORT            ON  CACHE INTERNAL "")
        set(INJA_BUILD_TESTS       OFF CACHE INTERNAL "")
        set(BUILD_TESTING          OFF CACHE INTERNAL "")
        set(BUILD_BENCHMARK        OFF CACHE INTERNAL "")
        set(COVERALLS              OFF CACHE INTERNAL "")
        add_subdirectory(${inja_SOURCE_DIR} ${inja_BINARY_DIR})
    endif()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MALLOY_TMP_BINDIR})    # Reset global var
