add_subdirectory(http)
add_subdirectory(routing)
add_subdirectory(websocket)

target_sources(
    ${TARGET_OBJS}
    PRIVATE
        controller.cpp
        controller.hpp
        listener.cpp
        listener.hpp
)
