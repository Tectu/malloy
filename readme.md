# Overview
Malloy is a small, embeddable HTTP & WebSocket server built on top of [boost.beast](https://www.boost.org/doc/libs/1_75_0/libs/beast/doc/html/index.html).

# Features
Currently implemented:
- HTTP Server
- WebSocket Server

Following soon™:
- Support for TLS connections

# Licensing
This library is MIT licensed. Dependencies ship with their own licensing models.

# Requirements
Building (and using) this library requires:
- A C++20 capable compiler
- CMake 3.17 or newer
  
## Dependencies
Malloy comes with a small number of dependencies:
- [Boost](https://www.boost.org/) 1.73.0 or newer
- [spdlog](https://github.com/gabime/spdlog) 1.8.3 or newer

# Motivation
This started off with the intention of creating a more complex, real-world example of how to use `boost.beast`.

# Documentation
Available documentation sources:
- API documentation (doxygen)

## Doxygen
The Doxygen API documentation can be generated with little hassle:
```shell
doxygen ./Doxyfile
```
The generated output(s) may be found under `/docs/doxygen`.