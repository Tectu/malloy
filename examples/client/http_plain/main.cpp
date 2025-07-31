#include "../../example.hpp"

#include <malloy/client/controller.hpp>

#include <iostream>

malloy::awaitable<void>
example()
{
    // Create the controller config
    malloy::client::controller::config cfg;
    cfg.logger      = create_example_logger();
    cfg.num_threads = 1;

    // Create the controller
    malloy::client::controller c{cfg};

    // Start
    [[maybe_unused]] auto session = start(c);

    // Make request
    auto resp = co_await c.http_request("http://www.google.com");
    if (!resp)
        spdlog::error("error: {}", resp.error().message());
    else
        std::cout << *resp << std::endl;
}

// Include main() which will invoke the example() coroutine
#include "../client_example_main.hpp"
