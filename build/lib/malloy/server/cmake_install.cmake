# Install script for directory: /home/kel/Desktop/malloy/lib/malloy/server

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kel/Desktop/malloy/build/lib/malloy/server/http/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kel/Desktop/malloy/build/lib/malloy/server/routing/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kel/Desktop/malloy/build/lib/malloy/server/websocket/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kel/Desktop/malloy/build/lib/malloy/server/auth/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/kel/Desktop/malloy/build/lib/malloy/server/libmalloy-server.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/malloy/server" TYPE FILE FILES
    "/home/kel/Desktop/malloy/lib/malloy/server/routing_context.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/server/listener.hpp"
    )
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/malloy/server/auth" TYPE FILE FILES "/home/kel/Desktop/malloy/lib/malloy/server/auth/basic.hpp")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/malloy/server/http" TYPE FILE FILES
    "/home/kel/Desktop/malloy/lib/malloy/server/http/connection.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/server/http/connection_detector.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/server/http/connection_plain.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/server/http/connection_t.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/server/http/connection_tls.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/server/http/preflight_config.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/server/http/request_generator_t.hpp"
    )
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/malloy/server/routing" TYPE FILE FILES
    "/home/kel/Desktop/malloy/lib/malloy/server/routing/endpoint.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/server/routing/endpoint_http.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/server/routing/endpoint_http_files.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/server/routing/endpoint_http_redirect.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/server/routing/endpoint_http_regex.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/server/routing/endpoint_websocket.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/server/routing/router.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/server/routing/type_traits.hpp"
    )
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/malloy/server/websocket" TYPE FILE FILES "/home/kel/Desktop/malloy/lib/malloy/server/websocket/connection.hpp")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/kel/Desktop/malloy/build/lib/malloy/server/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
