target_sources(
    ${TARGET}
    PUBLIC
        FILE_SET
            HEADERS
            BASE_DIRS ${MALLOY_CORE_BASE_DIR}
            FILES
                manager.hpp
                session.hpp
                storage.hpp
                storage_memory.hpp
                types.hpp

    PRIVATE
        manager.cpp
)
