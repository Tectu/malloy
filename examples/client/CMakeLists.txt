add_subdirectory(http_plain)
add_subdirectory(websocket_plain)
add_subdirectory(http-custom-filter)
add_subdirectory(http-file-download)

if (MALLOY_FEATURE_TLS)
    add_subdirectory(http_tls)
    add_subdirectory(websocket_secure)
endif()
if (MALLOY_BUILD_CORO_EXAMPLES)
    add_subdirectory(http-plain-coro)
endif()

