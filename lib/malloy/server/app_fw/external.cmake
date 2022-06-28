include(FetchContent)


########################################################################################################################
# nlohmann/json: https://github.com/nlohmann/json
########################################################################################################################
if (MALLOY_DEPENDENCY_JSON)
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
            set(JSON_Install OFF CACHE INTERNAL "")
            add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR})
        endif()
    endif()
endif()


########################################################################################################################
# pantor/inja: https://github.com/pantor/inja
########################################################################################################################
if (MALLOY_DEPENDENCY_INJA)
    FetchContent_Declare(
            inja
            GIT_REPOSITORY https://github.com/pantor/inja.git
            GIT_TAG        17a59c0b78e620157c253b0c7643cfa987135f8d
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
