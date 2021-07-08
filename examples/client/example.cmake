function(malloy_example_setup_client target)
    target_link_libraries(
        ${target}
        PRIVATE
            malloy-client
    )
endfunction()
