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
            BOOST_BEAST_USE_STD_STRING_VIEW
            SPDLOG_FMT_EXTERNAL
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
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/..
    )

    target_link_libraries(
        ${TARGET}
        PUBLIC
            spdlog::spdlog
            Boost::headers
            fmt::fmt
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
            RUNTIME_OUTPUT_DIRECTORY ${MALLOY_BINARY_DIR} 
            LIBRARY_OUTPUT_DIRECTORY ${MALLOY_BINARY_DIR}
    )

    include(GenerateExportHeader)
    generate_export_header(
        ${TARGET}
        BASE_NAME "malloy"
        EXPORT_FILE_NAME "malloy_export.hpp"
        DEPRECATED_MACRO_NAME "MALLOY_DEPRECATED"
        NO_DEPRECATED_MACRO_NAME "MALLOY_NO_DEPRECATED"
        EXPORT_MACRO_NAME "MALLOY_EXPORT"
        NO_EXPORT_MACRO_NAME "MALLOY_NO_EXPORT"
        STATIC_DEFINE "MALLOY_EXPORT_STATIC_DEFINE"
        DEFINE_NO_DEPRECATED
    )
endfunction()
