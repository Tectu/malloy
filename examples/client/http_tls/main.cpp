#include "../../example.hpp"

#include <boost/asio/use_future.hpp>
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
    if (!c.init_tls()) {
        std::cerr << "initializing TLS context failed." << std::endl;
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
        443,
        "/"
    );

    auto stop_token = c.https_request(req, boost::asio::use_future);
    try {
        std::cout << stop_token.get() << "\n";

    } catch(const std::future_error& e) {
        spdlog::error(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
