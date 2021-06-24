function(malloy_setup_defines TARGET)
  target_compile_options(
    ${TARGET} 
    PRIVATE 
        $<$<BOOL:${MINGW}>:-Wa,-mbig-obj> 
        $<$<BOOL:${MINGW}>:-O3>
        $<$<BOOL:${MSVC}>:/bigobj>
    )
  target_compile_definitions(
      ${TARGET} 
      PRIVATE 
        $<$<BOOL:${WIN32}>:BOOST_DATE_TIME_NO_LIB>
    )
endfunction()
