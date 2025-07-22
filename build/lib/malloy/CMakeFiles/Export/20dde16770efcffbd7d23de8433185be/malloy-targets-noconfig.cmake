#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "malloy::malloy-core" for configuration ""
set_property(TARGET malloy::malloy-core APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(malloy::malloy-core PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libmalloy-core.a"
  )

list(APPEND _cmake_import_check_targets malloy::malloy-core )
list(APPEND _cmake_import_check_files_for_malloy::malloy-core "${_IMPORT_PREFIX}/lib/libmalloy-core.a" )

# Import target "malloy::malloy-client" for configuration ""
set_property(TARGET malloy::malloy-client APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(malloy::malloy-client PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libmalloy-client.a"
  )

list(APPEND _cmake_import_check_targets malloy::malloy-client )
list(APPEND _cmake_import_check_files_for_malloy::malloy-client "${_IMPORT_PREFIX}/lib/libmalloy-client.a" )

# Import target "malloy::malloy-server" for configuration ""
set_property(TARGET malloy::malloy-server APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(malloy::malloy-server PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libmalloy-server.a"
  )

list(APPEND _cmake_import_check_targets malloy::malloy-server )
list(APPEND _cmake_import_check_files_for_malloy::malloy-server "${_IMPORT_PREFIX}/lib/libmalloy-server.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
