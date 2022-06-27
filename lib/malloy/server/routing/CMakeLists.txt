target_sources(
    ${TARGET}
    PUBLIC
        FILE_SET
            HEADERS
            BASE_DIRS ${MALLOY_SERVER_BASE_DIR}
            FILES
                endpoint.hpp
                endpoint_http.hpp
                endpoint_http_files.hpp
                endpoint_http_redirect.hpp
                endpoint_http_regex.hpp
                endpoint_websocket.hpp
                router.hpp
                type_traits.hpp

    PRIVATE
        router.cpp
)
