# Core
add_subdirectory(core)

# Client
if (MALLOY_FEATURE_CLIENT)
    add_subdirectory(client)
endif()

# Server
if (MALLOY_FEATURE_SERVER)
    add_subdirectory(server)
endif()
