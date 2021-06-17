# Include external dependencies
include(external.cmake)

set(NAME malloy)
set(TARGET_OBJS   ${NAME}-objs)

add_library(${TARGET_OBJS} OBJECT)

if (MALLOY_FEATURE_CLIENT)
    add_subdirectory(client)
endif()
add_subdirectory(http)
if (MALLOY_FEATURE_SERVER)
    add_subdirectory(server)
endif()
add_subdirectory(websocket)
if (MALLOY_FEATURE_HTML)
    add_subdirectory(html)
endif()
if (MALLOY_FEATURE_TLS)
    add_subdirectory(tls)
endif()

target_compile_features(
    ${TARGET_OBJS}
    PUBLIC
        cxx_std_20
)

target_compile_definitions(
    ${TARGET_OBJS}
    PUBLIC
        BOOST_BEAST_USE_STD_STRING_VIEW
        $<$<BOOL:${MALLOY_FEATURE_TLS}>:MALLOY_FEATURE_TLS>
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
        $<$<BOOL:${MALLOY_FEATURE_TLS}>:crypt32>        # ToDo: This is only needed when MALLOY_FEATURE_CLIENT is ON
        $<$<BOOL:${MALLOY_FEATURE_TLS}>:OpenSSL::Crypto>
        $<$<BOOL:${MALLOY_FEATURE_TLS}>:OpenSSL::SSL>
)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND WIN32 AND NOT MINGW)
    set(MSYS_GCC 1)
    message(STATUS "Assuming compiler is GCC on MSYS2")
endif()

target_compile_options(
    ${TARGET_OBJS}
    PRIVATE
    $<$<BOOL:${MSYS_GCC}>:-Wa,-mbig-obj>
    $<$<BOOL:${MSYS_GCC}>:-O3>
)
