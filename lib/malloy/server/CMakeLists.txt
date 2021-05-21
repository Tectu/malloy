add_subdirectory(routing)

target_sources(
    ${TARGET_OBJS}
    PRIVATE
        controller.cpp
        controller.hpp
        listener.cpp
        listener.hpp
)
