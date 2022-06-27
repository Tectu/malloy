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



########################################################################################################################
# Install                                                                                                              #
########################################################################################################################

include(CMakePackageConfigHelpers)


write_basic_package_version_file(
    malloy-config-version.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
)
install(
    FILES
        malloy-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/malloy-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/malloy
)

install(
    EXPORT malloy-targets
    FILE malloy-targets.cmake
    NAMESPACE malloy::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/malloy
)