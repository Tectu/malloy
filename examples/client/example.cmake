function(malloy_example_setup_client target)
    target_link_libraries(
        ${target}
        PRIVATE
            malloy-objs
    )

    target_compile_options(
        ${target}
        PRIVATE
            $<$<BOOL:${WIN32}>:-Wa,-mbig-obj>
            $<$<BOOL:${WIN32}>:-O3>
    )
endfunction()
