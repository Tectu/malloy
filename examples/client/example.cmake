function(malloy_example_setup_client target)
    target_link_libraries(
        ${target}
        PRIVATE
            malloy-objs
    )

    malloy_setup_defines(${target})
endfunction()
