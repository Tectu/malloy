<p align="center">
  <img height="180" src="https://raw.githubusercontent.com/Tectu/malloy/main/doc/logo.svg"/>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue.svg" alt="standard"/>
  <img src="https://img.shields.io/badge/License-BSD-blue.svg" alt="license"/>
  <a href="https://repology.org/project/malloy/versions">
    <img src="https://repology.org/badge/tiny-repos/malloy.svg" alt="Packaging status">
  </a>
</p>

# Packages
[![Packaging status](https://repology.org/badge/vertical-allrepos/malloy.svg)](https://repology.org/project/malloy/versions)

# Overview
Malloy is a small, embeddable HTTP & WebSocket server & client built on top of [boost](https://boost.org).

The main use case for this library is a C++ project which needs to embedd an HTTP and/or WebSocket server and/or client.

This library is being used successfully on:
- Windows (with both MSVC and MinGW)
- Linux (Ubuntu, Debian, Fedora, ...)
- MacOS
- FreeBSD

# Features
The following list provides an overview of the currently implemented features. Some of these are optional and can be enabled/disabled.

- High-level controller to setup I/O context, SSL context, worker threads and more
- HTTP
  - Plain or TLS (SSL) connections
  - Cookies
  - Sessions
  - Upgrading connections to WebSocket
  - Client
    - Response filters
  - Server
    - Routing
      - Simple handlers (useful for building REST APIs)
        - Target matching via regex
        - Capturing groups via regex
      - Sub-routers (nested/chained routers)
      - Redirections
      - File serving locations
        - Optional cache-control directives
      - Preflight responses
      - Access policies
        - HTTP basic auth
        - Custom access policies
      - Websocket endpoints (with auto-upgrade from HTTP)
    - Connection logging
    - Request filters
- WebSocket
  - Client
  - Server
  - Connections upgradable from HTTP
- HTML
  - Forms
    - Supported encoding types
      - `application/x-www-form-urlencoded`
      - `multipart/form-data`
      - `text/plain`
    - Parsing
    - Rendering

# Licensing
This library is BSD-3-Clause licensed. Dependencies ship with their own licensing models.

# Requirements
Building (and using) this library requires:
- A C++20 capable compiler
  - GCC >= 10.3 (MinGW works!)
  - Clang >= 13
  - MSVC >= 19.29 (VS 2019)
- CMake 3.23 or newer

## Dependencies
Required:
- [Boost](https://www.boost.org/) 1.79.0 or newer
- [spdlog](https://github.com/gabime/spdlog) 1.8.3 or newer
- [fmt](https://github.com/fmtlib/fmt) 7.1.3 or newer (must be compatible with spdlog version)

Optional:
- OpenSSL

# Examples
A variety of examples can be found in the `/examples` directory. You should definitely check those out! What follows are snippets for a simple HTTP server and a simple HTTP client.

HTTP Server:
```cpp
int main()
{
    // Create malloy controller config
    malloy::server::routing_context::config cfg;
    cfg.interface   = "127.0.0.1";
    cfg.port        = 8080;
    cfg.doc_root    = "/path/to/http/docs"
    cfg.num_threads = 5;
    cfg.logger      = std::make_shared<spdlog::logger>();

    // Create malloy controller
    malloy::server::routing_context c{cfg};

    // Create the router
    auto& router = c.router();
    {
        using namespace malloy::http;

        // A simple GET route handler
        router.add(method::get, "/", [](const auto& req) {
            response res{status::ok};
            res.body() = "<html><body><h1>Hello World!</h1><p>some content...</p></body></html>";
            return res;
        });

        // Add a route to an existing file
        router.add(method::get, "/file", [](const auto& req) {
            return generator::file(examples_doc_root, "index.html");
        });

        // Add a route to a non-existing file
        router.add(method::get, "/file_nonexist", [](const auto& req) {
            return generator::file(examples_doc_root, "/some_nonexisting_file.xzy");
        });

        // Add some redirections
        router.add_redirect(status::permanent_redirect, "/redirect1", "/");
        router.add_redirect(status::temporary_redirect, "/redirect2", "/");

        // Add some file serving
        router.add_file_serving("/files", examples_doc_root);

        // Add a websocket echo endpoint
        router.add_websocket("/echo", [](const auto& req, auto writer) {
            std::make_shared<malloy::examples::ws::server_echo>(writer)->run(req);
        });
    }

    // Start
    start(std::move(c)).run();

    return EXIT_SUCCESS;
}
```

HTTP client:
```cpp
int main()
{
    // Create the controller config
    malloy::client::controller::config cfg;
    cfg.num_threads = 1;
    cfg.logger      = create_example_logger();

    // Create the controller
    malloy::client::controller c{cfg};

    // Start
    [[maybe_unused]] auto session = start(c);

    // Create HTTP request
    malloy::http::request req(
        malloy::http::method::get,
        "www.google.com",
        80,
        "/"
    );
    
    // Make HTTP request
    auto stop_token = c.http_request(req, [](auto&& resp) mutable {
        std::cout << resp << std::endl;
    });
    const auto ec = stop_token.get();
    if (ec) {
        spdlog::error(ec.message());
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}
```

# Motivation
This started off with the intention of creating a more complex, real-world example of how to use `boost.beast`.

# Security
This is a work in progress and should generally be perceived as unfit for any production use.

As of today, no security research has been performed on this library.

Malloy is an open-source library provided WITHOUT any WARRANTY or GUARANTEE regarding security, safety, functionality or anything else. Use at your own risk!

# Documentation
Available documentation sources:
- API documentation (doxygen)

## Doxygen
The Doxygen API documentation can be generated with little hassle:
```shell
doxygen ./Doxyfile
```
The generated output(s) may be found under `/docs/doxygen`. Simply open `/docs/doxygen/html/index.html` in the web browser of your choosing.

# Integration
Malloy is designed to be an embeddable component for other C++ applications. As this is a CMake based project there are various ways to integrate this library
into another project:
- Via CMake's `FetchContent()`.
- Building the library locally, installing it on the system and using CMake's `find_package()`.
- Installing the corresponding [package for your OS/environment](https://repology.org/project/malloy/versions) and using CMake's `find_package()`.

## FetchContent()
The easiest way to integrate Malloy is via CMake's `FetchContent()` infrastructure:
```cmake
FetchContent_Declare(
    malloy
    GIT_REPOSITORY https://github.com/tectu/malloy
    GIT_TAG        main
)
FetchContent_MakeAvailable(malloy)
```
If you like to modify set some of Malloy's CMake variables, the `FetchContent_MakeAvailable()` call can be replaced accordingly:
```cmake
FetchContent_Declare(
    malloy
    GIT_REPOSITORY https://github.com/tectu/malloy
    GIT_TAG        main
)
FetchContent_GetProperties(malloy)
if(NOT malloy_POPULATED)
    FetchContent_Populate(malloy)
    
    # Change various malloy cmake options
    set(MALLOY_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(MALLOY_BUILD_TESTS OFF CACHE INTERNAL "")
    set(MALLOY_BUILD_SHARED ON CACHE INTERNAL "")
    set(MALLOY_FEATURE_CLIENT OFF CACHE INTERNAL "")
    set(MALLOY_FEATURE_SERVER ON CACHE INTERNAL "")
    set(MALLOY_FEATURE_TLS ON CACHE INTERNAL "")
    
    add_subdirectory(${malloy_SOURCE_DIR} ${malloy_BINARY_DIR})
endif()
```
You may replace `GIT_TAG` with a commit hash or a release tag such as `1.0.0`.

After fetching the content, it's only a matter of linking the malloy library target(s) to the consuming application:
```cmake
target_link_libraries(
    my_application
    PRIVATE
        malloy-server       # Link malloy's server components
        malloy-client       # Link malloy's client components
)
```
Where `my_application` is your application (or library) target that should consume malloy.

## Options
Various `cmake` options are available to control the build:

### Build
| Option                  | Default | Description                                                                         |
|-------------------------|---------|-------------------------------------------------------------------------------------|
| `MALLOY_BUILD_EXAMPLES` | `ON`    | Whether to build examples.                                                          |
| `MALLOY_BUILD_TESTS`    | `ON`    | Whether to build the test suite(s).                                                 |
| `MALLOY_BUILD_SHARED`   | `OFF`   | Whether to build shared libraries. If set to `OFF`, static libraries will be built. |

### Features
| Option                  | Default | Description                                                         |
|-------------------------|---------|---------------------------------------------------------------------|
| `MALLOY_FEATURE_CLIENT` | `ON`    | Enable client features.                                             |
| `MALLOY_FEATURE_SERVER` | `ON`    | Enable server features.                                             |
| `MALLOY_FEATURE_HTML`   | `ON`    | Whether to enable HTML support.                                     |
| `MALLOY_FEATURE_TLS`    | `OFF`   | Whether to enable TLS support.                                      |

### Dependencies
| Option                              | Default | Description                                                                                             |
|-------------------------------------|---------|---------------------------------------------------------------------------------------------------------|
| `MALLOY_DEPENDENCY_SPDLOG_DOWNLOAD` | `ON`    | Whether to use `FetchContent()` to pull in `spdlog`. If set to `OFF`, `find_package()` is used instead. |
| `MALLOY_DEPENDENCY_FMT_DOWNLOAD`    | `ON`    | Same as above but for `fmt`                                                                             |
