target_sources(
    ${TARGET}
    PUBLIC
        FILE_SET
            HEADERS
            BASE_DIRS ${MALLOY_CORE_BASE_DIR}
            FILES
                form.hpp
                form_renderer.hpp
                html.hpp
                multipart_parser.hpp

    PRIVATE
        form.cpp
        form_renderer.cpp
        multipart_parser.cpp
)
