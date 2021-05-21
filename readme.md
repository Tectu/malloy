# Overview
Malloy is a small, embeddable HTTP & WebSocket server built on top of [boost.beast](https://www.boost.org/doc/libs/1_75_0/libs/beast/doc/html/index.html).

The main use case for this library is a C++ project which needs to embedd an HTTP and/or WebSocket server.

# Features
The following list provides an overview of the currently implemented features. Some of these are optional and can be enabled/disabled.

- Server
  - Plain or TLS (SSL) connections
  - HTTP
    - Sessions
  - Websocket
  - Request router
    - Simple handlers (useful for building REST APIs)
    - Sub-routers (nested routers)
    - Redirections
    - File serving locations
    - Automatic preflight responses (optional)
  - High-level controller class to setup I/O context, SSL context, worker threads and more.
- Cookies
- HTML
  - Parsing HTML forms

Coming soon™:
- Security middleware (CSRF, XSS, ...)
- HTTP client
- Websocket client

# Licensing
This library is MIT licensed. Dependencies ship with their own licensing models.

# Requirements
Building (and using) this library requires:
- A C++20 capable compiler
- CMake 3.17 or newer
  
## Dependencies
Required:
- [Boost](https://www.boost.org/) 1.73.0 or newer
- [spdlog](https://github.com/gabime/spdlog) 1.8.3 or newer

Optional:
- OpenSSL

# Examples
A variety of examples can be found in the `/examples` directory.

The most basic use looks like this:
```cpp
int main()
{
    const std::filesystem::path doc_root = "../../../../examples/static_content";

    // Create malloy controller config
    malloy::server::controller::config cfg;
    cfg.interface   = "127.0.0.1";
    cfg.port        = 8080;
    cfg.doc_root    = doc_root;
    cfg.num_threads = 5;

    // Create malloy controller
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
        router->add(method::get, "/file", [doc_root](const auto& req) {
            return generator::file(doc_root, "index.html");
        });

        // Add some file serving
        router->add_file_serving("/files", doc_root);
    }

    // Start
    c.start();

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
    set(BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(BUILD_TESTS OFF CACHE INTERNAL "")
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
| `MALLOY_FEATURE_HTML` | `ON` | Whether to enable HTML support. |
| `MALLOY_FEATURE_TLS` | `OFF` | Whether to enable TLS support. |
