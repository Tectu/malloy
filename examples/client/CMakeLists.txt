add_subdirectory(http_assembled_request)
add_subdirectory(http_custom_filter)
add_subdirectory(http_file_download)
add_subdirectory(http_plain)
add_subdirectory(websocket)

if (MALLOY_FEATURE_TLS)
    add_subdirectory(http_tls)
endif()

