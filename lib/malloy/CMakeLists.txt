# Include external dependencies
include(external.cmake)

set(NAME malloy)
set(TARGET_OBJS   ${NAME}-objs)

add_library(${TARGET_OBJS} OBJECT)

add_subdirectory(http)
add_subdirectory(websocket)

target_compile_features(
    ${TARGET_OBJS}
    PUBLIC
        cxx_std_20
)

target_compile_definitions(
    ${TARGET_OBJS}
    PUBLIC
        BOOST_BEAST_USE_STD_STRING_VIEW
        $<$<BOOL:${WIN32}>:UNICODE>
        $<$<BOOL:${WIN32}>:_UNICODE>
        $<$<BOOL:${WIN32}>:WIN32_LEAN_AND_MEAN>
)

target_include_directories(
    ${TARGET_OBJS}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/..
        ${gpds_SOURCE_DIR}/lib/include
        ${gpds_BINARY_DIR}/lib
)

target_sources(
    ${TARGET_OBJS}
    PRIVATE
        controller.cpp
        controller.hpp
        listener.cpp
        listener.hpp
        malloy.hpp
        utils.hpp
)

target_link_libraries(
    ${TARGET_OBJS}
    PUBLIC
        spdlog::spdlog
    PRIVATE
        $<$<BOOL:${WIN32}>:wsock32>
        $<$<BOOL:${WIN32}>:ws2_32>
)

target_compile_options(
    ${TARGET_OBJS}
    PRIVATE
        $<$<BOOL:${WIN32}>:-Wa,-mbig-obj>
        $<$<BOOL:${WIN32}>:-O3>
)
