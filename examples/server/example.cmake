function(malloy_example_setup target)
    target_link_libraries(
        ${target}
        PRIVATE
            malloy-server
    )
endfunction()
