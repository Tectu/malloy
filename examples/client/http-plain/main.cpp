#include "../../example_logger.hpp"

#include <malloy/client/controller.hpp>
#include <malloy/client/http/connection_plain.hpp>

#include <iostream>

int main()
{
    // Create the controller config
    malloy::client::controller::config cfg;
    cfg.num_threads = 1;
    cfg.logger      = create_example_logger();

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

    malloy::http::request req(
        malloy::http::method::get,
        "www.google.com",
        80,
        "/"
    );
    auto resp = c.http_request<malloy::client::http::connection_plain>(req);

    std::cout << resp.get() << std::endl;

    return EXIT_SUCCESS;
}
