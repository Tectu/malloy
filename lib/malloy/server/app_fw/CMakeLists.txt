include(external.cmake)

target_sources(
    ${TARGET}
    PUBLIC
        FILE_SET
            HEADERS
            BASE_DIRS ${MALLOY_SERVER_BASE_DIR}
            FILES
                app.hpp
                environment.hpp
                page.hpp
                page_content.hpp
                page_master.hpp

    PRIVATE
        app.cpp
        page_content.cpp
        page_master.cpp
)

target_link_libraries(
    ${TARGET}
    PUBLIC
        pantor::inja
        nlohmann_json::nlohmann_json
)

install(
    TARGETS
        inja
        nlohmann_json
    EXPORT malloy-targets
)
