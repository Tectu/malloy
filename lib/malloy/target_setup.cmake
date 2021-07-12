function(malloy_target_common_setup TARGET)
    target_compile_features(
        ${TARGET}
        PUBLIC
            cxx_std_20
    )

    target_compile_options(
        ${TARGET}
        PUBLIC
            $<$<BOOL:${MINGW}>:-Wa,-mbig-obj>
            $<$<BOOL:${MINGW}>:-O3>
            $<$<BOOL:${MSVC}>:/bigobj>
    )
    if (MSVC) 
        target_compile_options(
            ${TARGET} 
            PRIVATE 
                /Wall
                /WX
            )
    elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "(AppleClang|Clang|GNU)")
    target_compile_options( 
        ${TARGET}
        PRIVATE 
            -Wall 
            -Wextra 
            -Wpedantic 
            -Werror
        )
        
    endif()


    target_compile_definitions(
        ${TARGET}
        PUBLIC
            BOOST_BEAST_USE_STD_STRING_VIEW
            $<$<BOOL:${MALLOY_FEATURE_TLS}>:MALLOY_FEATURE_TLS>
            $<$<BOOL:${WIN32}>:UNICODE>
            $<$<BOOL:${WIN32}>:_UNICODE>
            $<$<BOOL:${WIN32}>:WIN32_LEAN_AND_MEAN>
            $<$<BOOL:${WIN32}>:BOOST_DATE_TIME_NO_LIB>
    )

    target_include_directories(
        ${TARGET}
        PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/..
    )

    target_link_libraries(
        ${TARGET}
        PUBLIC
            spdlog::spdlog
            Boost::headers
            $<$<BOOL:${MALLOY_FEATURE_TLS}>:OpenSSL::Crypto>
            $<$<BOOL:${MALLOY_FEATURE_TLS}>:OpenSSL::SSL>
            $<$<AND:$<BOOL:${MALLOY_FEATURE_TLS}>,$<BOOL:${WIN32}>>:crypt32>        # ToDo: This is only needed when MALLOY_FEATURE_CLIENT is ON
        PRIVATE
            $<$<BOOL:${WIN32}>:wsock32>
            $<$<BOOL:${WIN32}>:ws2_32>
    )
endfunction()
