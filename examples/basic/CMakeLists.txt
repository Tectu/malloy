# Set a target name
set(TARGET malloy-example-basic)

# Create the executable
add_executable(${TARGET})

target_sources(
    ${TARGET}
    PRIVATE
        main.cpp
)

target_link_libraries(
    ${TARGET}
    PRIVATE
        malloy-objs
)

target_compile_options(
    ${TARGET}
    PRIVATE
        -Wa,-mbig-obj
        -O3
)
