target_sources(
    ${TARGET_OBJS}
    PRIVATE
        connection_plain.hpp
        connection_tls.hpp
        controller.cpp
        controller.hpp
)

target_include_directories(
    ${TARGET_OBJS}
    PRIVATE
        3rdparty
)
