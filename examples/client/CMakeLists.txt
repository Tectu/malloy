add_subdirectory(http-plain)
add_subdirectory(websocket-plain)
add_subdirectory(http-file-download)
add_subdirectory(http-custom-filter)

if (MALLOY_FEATURE_TLS)
    add_subdirectory(http_tls)
endif()

