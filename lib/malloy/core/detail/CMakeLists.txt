target_sources(
    ${TARGET}
    PUBLIC
        FILE_SET
            HEADERS
            BASE_DIRS ${MALLOY_CORE_BASE_DIR}
            FILES
                action_queue.hpp
                version_checks.hpp
                controller_run_result.hpp
)
