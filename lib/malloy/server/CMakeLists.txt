include(external.cmake)
include(../target_setup.cmake)

# Define target name
set(TARGET "malloy-server")

# Create library
add_library(${TARGET} ${MALLOY_LIBRARY_TYPE})

# Apply common setup
malloy_target_common_setup(${TARGET})

# Add subdirectories
add_subdirectory(http)
add_subdirectory(routing)
add_subdirectory(websocket)
if (MALLOY_FEATURE_APPFW)
    add_subdirectory(app_fw)
endif()

# Add sources
target_sources(
    ${TARGET}
    PUBLIC
        controller.hpp
        listener.hpp
    PRIVATE
        controller.cpp
        listener.cpp
)

# Link libraries
target_link_libraries(
    ${TARGET}
    PUBLIC
        malloy-core
        $<$<BOOL:${MALLOY_FEATURE_APPFW}>:pantor::inja>
        $<$<BOOL:${MALLOY_FEATURE_APPFW}>:nlohmann_json::nlohmann_json>
)
