target_sources(
    ${TARGET}
    PUBLIC
        FILE_SET
            HEADERS
            BASE_DIRS ${MALLOY_CORE_BASE_DIR}
            FILES
                connection.hpp
                stream.hpp
                types.hpp
                url.hpp
                websocket.hpp
)
