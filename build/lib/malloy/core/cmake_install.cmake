# Install script for directory: /home/kel/Desktop/malloy/lib/malloy/core

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
  include("/home/kel/Desktop/malloy/build/lib/malloy/core/html/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kel/Desktop/malloy/build/lib/malloy/core/http/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kel/Desktop/malloy/build/lib/malloy/core/tcp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kel/Desktop/malloy/build/lib/malloy/core/websocket/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kel/Desktop/malloy/build/lib/malloy/core/detail/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/kel/Desktop/malloy/build/lib/malloy/core/libmalloy-core.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/malloy/core" TYPE FILE FILES
    "/home/kel/Desktop/malloy/lib/malloy/core/controller.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/error.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/mp.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/type_traits.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/utils.hpp"
    )
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/malloy/core/detail" TYPE FILE FILES
    "/home/kel/Desktop/malloy/lib/malloy/core/detail/action_queue.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/detail/controller_run_result.hpp"
    )
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/malloy/core/html" TYPE FILE FILES
    "/home/kel/Desktop/malloy/lib/malloy/core/html/form.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/html/form_data.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/html/form_field.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/html/form_renderer.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/html/html.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/html/multipart_parser.hpp"
    )
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/malloy/core/http" TYPE FILE FILES
    "/home/kel/Desktop/malloy/lib/malloy/core/http/cookie.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/http/generator.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/http/http.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/http/request.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/http/response.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/http/type_traits.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/http/types.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/http/url.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/http/utils.hpp"
    )
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/malloy/core/http/filters" TYPE FILE FILES "/home/kel/Desktop/malloy/lib/malloy/core/http/filters/file.hpp")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/malloy/core/http/session" TYPE FILE FILES
    "/home/kel/Desktop/malloy/lib/malloy/core/http/session/manager.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/http/session/session.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/http/session/storage.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/http/session/storage_memory.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/http/session/types.hpp"
    )
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/malloy/core/tcp" TYPE FILE FILES
    "/home/kel/Desktop/malloy/lib/malloy/core/tcp/rate_policy.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/tcp/stream.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/tcp/tcp.hpp"
    )
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/malloy/core/websocket" TYPE FILE FILES
    "/home/kel/Desktop/malloy/lib/malloy/core/websocket/connection.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/websocket/stream.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/websocket/types.hpp"
    "/home/kel/Desktop/malloy/lib/malloy/core/websocket/websocket.hpp"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/kel/Desktop/malloy/build/lib/malloy/core/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
