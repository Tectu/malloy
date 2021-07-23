include(FetchContent)

########################################################################################################################
# Boost
########################################################################################################################
find_package(
    Boost
    1.74.0
    REQUIRED
)

if (MALLOY_DEPENDENCY_FMT_DOWNLOAD) 
    FetchContent_Declare( 
        fmt 
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG v7.1.3 # Highest supported by spdlog 1.8.3 
    )
    FetchContent_MakeAvailable(fmt)

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
        GIT_TAG        v1.8.3
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
