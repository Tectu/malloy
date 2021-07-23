include(FetchContent)

########################################################################################################################
# Boost
########################################################################################################################
find_package(
    Boost
    1.74.0
    REQUIRED
)

########################################################################################################################
# spdlog
########################################################################################################################
if (MALLOY_DEPENDENCY_SPDLOG_DOWNLOAD)

    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog
        GIT_TAG        v1.8.3
    )
    FetchContent_GetProperties(spdlog)
    if (NOT spdlog_POPULATED) 
        FetchContent_Populate(spdlog)
        set(SPDLOG_BUILD_SHARED ${MALLOY_BUILD_SHARED} CACHE INTERNAL "")

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
