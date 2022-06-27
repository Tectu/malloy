target_sources(
    ${TARGET}
    PUBLIC
        FILE_SET
            HEADERS
            BASE_DIRS ${MALLOY_CLIENT_BASE_DIR}
            FILES
                connection.hpp
                connection_plain.hpp
                connection_tls.hpp
)
