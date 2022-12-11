include(FetchContent)

set(MALLOY_TMP_BINDIR ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MALLOY_BINARY_DIR})


########################################################################################################################
# Boost
########################################################################################################################
find_package(
    Boost
    ${MALLOY_DEPENDENCY_BOOST_VERSION_MIN}
    REQUIRED
)


########################################################################################################################
# fmt
########################################################################################################################
if (MALLOY_DEPENDENCY_FMT_DOWNLOAD)
    FetchContent_Declare(
        fmt 
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG 9.1.0 # Supported by spdlog 1.11.0
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
        GIT_TAG        v1.x
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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MALLOY_TMP_BINDIR})    # Reset global var
