target_sources(
    ${TARGET}
    PUBLIC
        FILE_SET
            HEADERS
            BASE_DIRS ${MALLOY_CORE_BASE_DIR}
            FILES
                manager.hpp

    PRIVATE
        manager.cpp
)
