![](./doc/logo.svg)

# Overview
Malloy is a small, embeddable HTTP & WebSocket server & client built on top of [boost.beast](https://www.boost.org/doc/libs/1_75_0/libs/beast/doc/html/index.html).

The main use case for this library is a C++ project which needs to embedd an HTTP and/or WebSocket server and/or client.

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
      - Preflight responses
      - Access policies
        - HTTP basic auth
        - Custom access policies
      - Websocket endpoints (with auto-upgrade from HTTP)
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
This library is MIT licensed. Dependencies ship with their own licensing models.

# Requirements
Building (and using) this library requires:
- A C++20 capable compiler
  - GCC >= 10.3 (MinGW works!)
  - Clang >= 13
  - MSVC >= 19.29 (VS 2019)
- CMake 3.17 or newer

As of today, this library was tested successfully on:
- Windows 10 (with both MSVC and MinGW)
- Linux (Ubuntu, Debian, Fedora, ...)
- FreeBSD

## Dependencies
Required:
- [Boost](https://www.boost.org/) 1.74.0 or newer
- [spdlog](https://github.com/gabime/spdlog) 1.8.3 or newer
- [fmt](https://github.com/fmtlib/fmt) 7.1.3 or newer (must be compatible with spdlog version)

Optional:
- OpenSSL
- [nlohmann/json](https://github.com/nlohmann/json)
- [pantor/inja](https://github.com/pantor/inja)

# Examples
A variety of examples can be found in the `/examples` directory. You should definitely check those out! What follows are snippets for a simple HTTP server and a simple HTTP client.

HTTP Server:
```cpp
int main()
{
    // Create controller config
    malloy::server::controller::config cfg;
    cfg.interface   = "127.0.0.1";
    cfg.port        = 8080;
    cfg.doc_root    = "/path/to/http/docs"
    cfg.num_threads = 5;
    cfg.logger      = std::make_shared<spdlog::logger>();

    // Create controller
    malloy::server::controller c;
    if (!c.init(cfg)) {
        std::cerr << "could not start controller." << std::endl;
        return EXIT_FAILURE;
    }

    // Create the router
    auto router = c.router();
    if (router) {
        using namespace malloy::http;

        // A simple GET route handler
        router->add(method::get, "/", [](const auto& req) {
            response res{status::ok};
            res.body() = "<html><body><h1>Hello World!</h1><p>some content...</p></body></html>";
            return res;
        });

        // Add a route to an existing file
        router->add(method::get, "/file", [](const auto& req) {
            return generator::file(examples_doc_root, "index.html");
        });

        // Add a route to a non-existing file
        router->add(method::get, "/file_nonexist", [](const auto& req) {
            return generator::file(examples_doc_root, "/some_nonexisting_file.xzy");
        });

        // Add some redirections
        router->add_redirect(status::permanent_redirect, "/redirect1", "/");
        router->add_redirect(status::temporary_redirect, "/redirect2", "/");

        // Add some file serving
        router->add_file_serving("/files", examples_doc_root);

        // Add a websocket echo endpoint
        router->add_websocket("/echo", [](const auto& req, auto writer) {
            std::make_shared<malloy::examples::ws::server_echo>(writer)->run(req);
        });
    }

    // Start
    c.start();

    // Keep the application alive
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

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
    cfg.logger      = std::make_shared<spdlog::logger>();

    // Create the controller
    malloy::client::controller c;
    if (!c.init(cfg)) {
        std::cerr << "initializing controller failed." << std::endl;
        return EXIT_FAILURE;
    }

    // Start
    if (!c.start()) {
        std::cerr << "starting controller failed." << std::endl;
        return EXIT_FAILURE;
    }

    // Prepare the requst
    malloy::http::request req(
        malloy::http::method::get,
        "www.google.com",
        80,
        "/"
    );

    // Make the request
    auto stop_token = c.http_request(req, [](auto&& resp) mutable {
        // Print the HTTP response
        std::cout << resp << std::endl;
    });
    const auto ec = stop_token.get();
    if (ec) {
        std::cerr << "error making HTTP request: " << ec.message() << std::endl;
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
Malloy is designed to be an embeddable component for other C++ applications.
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
    set(MALLOY_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(MALLOY_BUILD_TESTS OFF CACHE INTERNAL "")
    set(MALLOY_BUILD_SHARED ON CACHE INTERNAL "")
    set(MALLOY_FEATURE_CLIENT OFF CACHE INTERNAL "")
    set(MALLOY_FEATURE_SERVER ON CACHE INTERNAL "")
    set(MALLOY_FEATURE_TLS ON CACHE INTERNAL "")
    add_subdirectory(${malloy_SOURCE_DIR} ${malloy_BINARY_DIR})
endif()
```
You may replace `GIT_TAG` with a commit hash or a release tag such as `v1.0.0`.

After fetching the content, it's only a matter of linking the malloy library target to the client application:
```cmake
target_link_libraries(
    my_application
    PRIVATE
        malloy-objs
)
```
Where `my_application` is your application (or library) target that should consume malloy.

## Options
Various `cmake` options are available to control the build:

| Option | Default | Description |
| --- | --- | --- |
| `MALLOY_BUILD_EXAMPLES` | `ON` | Whether to build examples. |
| `MALLOY_BUILD_TESTS` | `ON` | Whether to build the test suite(s). |
| `MALLOY_BUILD_SHARED` | `OFF` | Whether to build shared libraries. If set to `OFF`, static libraries will be built.  |
| `MALLOY_FEATURE_CLIENT` | `ON` | Enable client features. |
| `MALLOY_FEATURE_SERVER` | `ON` | Enable server features. |
| `MALLOY_FEATURE_HTML` | `ON` | Whether to enable HTML support. |
| `MALLOY_FEATURE_TLS` | `OFF` | Whether to enable TLS support. |
| `MALLOY_DEPENDENCY_SPDLOG_DOWNLOAD` | `ON` | Whether to use `FetchContent()` to pull in `spdlog`. If set to `OFF`, `find_package()` is used instead. |
| `MALLOY_DEPENDENCY_FMT_DOWNLOAD` | `ON` | Same as above but for `fmt` |
| `MALLOY_DEPENDENCY_JSON_DOWNLOAD` | `ON` | Same as above but for `[nlohmann/json](https://github.com/nlohmann/json)` |
| `MALLOY_DEPENDENCY_INJA_DOWNLOAD` | `ON` | Same as above but for `[pantor/inja](https://github.com/pantor/inja)` |
