#include <malloy/client/controller.hpp>

#include <iostream>

int main()
{
    // Create the controller config
    malloy::client::controller::config cfg;
    cfg.num_threads = 1;

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

    c.add_connection(
        "id[0]",
        "127.0.0.1",
        8080,
        "/echo",
        [](const auto& foo, auto writer) {
            std::cout << "id[0]: " << foo << std::endl;
        }
    );

    c.add_connection(
        "id[1]",
        "127.0.0.1",
        8080,
        "/timer",
        [](const auto& foo, auto writer) {
            std::cout << "id[1]: " << foo << std::endl;
        }
    );

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(15s);

    // Stop
    c.stop().wait();

    return EXIT_SUCCESS;
}
