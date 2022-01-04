# Ensure that both server and client are enabled
if (NOT (MALLOY_FEATURE_SERVER AND MALLOY_FEATURE_CLIENT))
    message(FATAL_ERROR "Both client and server components must be enabled when building the test suite.")
endif()

set(TARGET malloy-tests)

add_executable(${TARGET})

add_subdirectory(test_suites)

target_sources(
    ${TARGET}
    PRIVATE
        test_main.cpp
        test.hpp
        mocks.hpp
        tls_data.hpp
)

target_link_libraries(
    ${TARGET}
    PRIVATE
        malloy-client
        malloy-server
)

###
# CTest
###
enable_testing()

add_test(
    NAME doctest
    COMMAND ${TARGET}
)

set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MALLOY_BINARY_DIR})

if (MSVC AND MALLOY_INTERNAL_CIMODE)
    target_compile_options(
        ${TARGET}
        PRIVATE
            /Zm200
        )
endif()
