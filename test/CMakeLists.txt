set(TARGET malloy-tests)

add_executable(${TARGET})

add_subdirectory(test_suites)

target_include_directories(
    ${TARGET}
    PRIVATE
        3rdparty/doctest
        "${CMAKE_SOURCE_DIR}/lib"
)

target_sources(
    ${TARGET}
    PRIVATE
        test_main.cpp
        test.hpp
)

target_link_libraries(
    ${TARGET}
    PRIVATE
        malloy-objs
)

###
# CTest
###
enable_testing()

add_test(
    NAME doctest
    COMMAND ${TARGET}
)
