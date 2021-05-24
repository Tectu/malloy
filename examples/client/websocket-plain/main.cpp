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

    c.test_plain();

    // Stop
    c.stop().wait();

    return EXIT_SUCCESS;
}
