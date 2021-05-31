#include "../../example_logger.hpp"

#include <malloy/client/controller.hpp>
#include <malloy/client/websocket/connection_plain.hpp>

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

    c.make_websocket_connection<malloy::client::websocket::connection_plain>(
        "127.0.0.1",
        8080,
        "/echo",
        [](const auto& foo, auto send) {
            std::cout << "id[0]: " << foo << std::endl;
        }
    )->send("Hello from Malloy!");

    c.make_websocket_connection<malloy::client::websocket::connection_plain>(
        "127.0.0.1",
        8080,
        "/timer",
        [](const auto& foo, auto send) {
            std::cout << "id[1]: " << foo << std::endl;
        }
    )->send("Whoop Whoop");

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(15s);

    // Stop
    c.stop().wait();

    return EXIT_SUCCESS;
}
