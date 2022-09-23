target_sources(
    ${TARGET}
    PUBLIC
        FILE_SET
            HEADERS
            BASE_DIRS ${MALLOY_CORE_BASE_DIR}
            FILES
                concepts.hpp
                message.hpp
                object.hpp
                rest.hpp
)
