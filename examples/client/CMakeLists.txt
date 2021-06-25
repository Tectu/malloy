add_subdirectory(http_plain)
add_subdirectory(websocket_plain)

if (MALLOY_FEATURE_TLS)
    add_subdirectory(http_tls)
endif()

