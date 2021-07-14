function(malloy_example_setup target)
    target_link_libraries(
        ${target}
        PRIVATE
            malloy-server
    )

    set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MALLOY_BINARY_DIR})
endfunction()
