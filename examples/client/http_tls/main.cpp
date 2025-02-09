#include "../../example.hpp"

#include <malloy/client/controller.hpp>

#include <iostream>

int main()
{
    // Create the controller config
    malloy::client::controller::config cfg;
    cfg.num_threads = 1;
    cfg.logger      = create_example_logger();

    // Create the controller
    malloy::client::controller c{cfg};

    if (!c.init_tls()) {
        std::cerr << "initializing TLS context failed." << std::endl;
        return EXIT_FAILURE;
    }

    // Start
    [[maybe_unused]] auto session = start(c);

    // Make request
    auto stop_token = c.http_request(
        malloy::http::method::get,
        "https://www.google.com",
        [](auto&& resp) mutable {
            std::cout << resp << std::endl;
    });
    const auto ec = stop_token.get();
    if (ec) {
        spdlog::error("error: {}", ec.message());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
