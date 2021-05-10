add_subdirectory(basic)
add_subdirectory(html_form)
add_subdirectory(nested_routers)
add_subdirectory(session)
add_subdirectory(websocket)

if (FEATURE_TLS)
    add_subdirectory(ssl)
endif()
