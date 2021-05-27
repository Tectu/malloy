add_subdirectory(websocket)

target_sources(
    ${TARGET_OBJS}
    PRIVATE
        controller.cpp
        controller.hpp
)

target_include_directories(
    ${TARGET_OBJS}
    PRIVATE
        3rdparty
)
