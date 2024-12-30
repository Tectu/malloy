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
    set(MALLOY_TMP_VAR1 ${BUILD_SHARED_LIBS})
    set(BUILD_SHARED_LIBS ${MALLOY_BUILD_SHARED}) # fmt has no specific shared option

    FetchContent_Declare(
        fmt 
        GIT_REPOSITORY  https://github.com/fmtlib/fmt
        GIT_TAG         10.1.1
    )
    FetchContent_MakeAvailable(fmt)

    set(BUILD_SHARED_LIBS ${MALLOY_TMP_VAR1})
else()
    find_package(fmt REQUIRED)
endif()


########################################################################################################################
# spdlog
########################################################################################################################
if (MALLOY_DEPENDENCY_SPDLOG_DOWNLOAD)
    set(SPDLOG_BUILD_SHARED ${MALLOY_BUILD_SHARED} CACHE INTERNAL "")
    set(SPDLOG_FMT_EXTERNAL ON CACHE INTERNAL "")

    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog
        GIT_TAG        v1.x
    )
    FetchContent_MakeAvailable(spdlog)
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
