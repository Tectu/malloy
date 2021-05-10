add_subdirectory(connection)

target_sources(
    ${TARGET_OBJS}
    PRIVATE
        types.hpp
        websocket.hpp
)
