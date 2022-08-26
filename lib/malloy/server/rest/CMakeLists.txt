target_sources(
    ${TARGET}
    PUBLIC
        FILE_SET
            HEADERS
            BASE_DIRS ${MALLOY_SERVER_BASE_DIR}
            FILES
                # TEMPORARY
                3rdparty/nlohmann/json.hpp
                3rdparty/nlohmann/json_fwd.hpp

                concepts.hpp
                message.hpp
                resource.hpp
                rest.hpp

    PRIVATE
        message.cpp
)
