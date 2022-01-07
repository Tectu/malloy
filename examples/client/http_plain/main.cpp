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

    // Start
    [[maybe_unused]] auto session = start(c);

    // Create request
    malloy::http::request req(
        malloy::http::method::get,
        "www.google.com",
        80,
        "/"
    );

    // Make request
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
