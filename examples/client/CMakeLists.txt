add_subdirectory(http-plain)
add_subdirectory(websocket-plain)

if (MALLOY_FEATURE_TLS)
    add_subdirectory(http_tls)
endif()

