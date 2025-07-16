function(malloy_target_common_setup TARGET)
    target_compile_features(
        ${TARGET}
        PUBLIC
            cxx_std_23
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
                /W4
                /WX
                /external:anglebrackets # Disable warnings for dependencies
                /external:W0
            )
    elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "(AppleClang|Clang|GNU)" AND NOT MINGW)
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
            $<$<BOOL:${MALLOY_FEATURE_HTML}>:MALLOY_FEATURE_HTML>
            $<$<BOOL:${MALLOY_FEATURE_TLS}>:MALLOY_FEATURE_TLS>
            $<$<BOOL:${WIN32}>:UNICODE>
            $<$<BOOL:${WIN32}>:_UNICODE>
            $<$<BOOL:${WIN32}>:WIN32_LEAN_AND_MEAN>
            $<$<BOOL:${WIN32}>:BOOST_DATE_TIME_NO_LIB>
    )
    if (MALLOY_LIBRARY_TYPE STREQUAL "STATIC")
        target_compile_definitions(
            ${TARGET}
            PUBLIC
                MALLOY_EXPORT_STATIC_DEFINE
        )
    endif()

    target_include_directories(
        ${TARGET}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_FUNCTION_LIST_DIR}/..>
    )

    target_link_libraries(
        ${TARGET}
        PUBLIC
            spdlog::spdlog
            Boost::headers
            $<$<BOOL:${MALLOY_FEATURE_TLS}>:OpenSSL::Crypto>
            $<$<BOOL:${MALLOY_FEATURE_TLS}>:OpenSSL::SSL>
            $<$<AND:$<BOOL:${MALLOY_FEATURE_TLS}>,$<BOOL:${WIN32}>>:crypt32>        # ToDo: This is only needed when MALLOY_FEATURE_CLIENT is ON
            $<$<BOOL:${WIN32}>:ws2_32>
        PRIVATE
            $<$<BOOL:${WIN32}>:wsock32>
    )

    set_target_properties(
        ${TARGET} 
        PROPERTIES
            VERSION ${PROJECT_VERSION}
            RUNTIME_OUTPUT_DIRECTORY ${MALLOY_BINARY_DIR} 
            LIBRARY_OUTPUT_DIRECTORY ${MALLOY_BINARY_DIR}
    )
endfunction()
