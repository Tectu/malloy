#include "../../example_logger.hpp"

#include <malloy/client/controller.hpp>

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
    std::promise<void> stop_prom;
    auto stop_token = stop_prom.get_future();
    c.http_request(req, [stop_prom = std::move(stop_prom)](auto&& resp) mutable {
        std::cout << resp << std::endl;
        stop_prom.set_value();
    });
    stop_token.wait();


    return EXIT_SUCCESS;
}
