#include "malloy/controller.hpp"

#include <iostream>
#include <memory>

int main()
{
    const std::filesystem::path doc_root = "../../../../examples/server/static_content";

    // Create malloy controller config
    malloy::server::controller::config cfg;
    cfg.interface   = "127.0.0.1";
    cfg.port        = 8080;
    cfg.doc_root    = doc_root;
    cfg.num_threads = 5;

    // Create malloy controller
    malloy::server::controller c;
    if (not c.init(cfg)) {
        std::cerr << "could not start controller." << std::endl;
        return EXIT_FAILURE;
    }

    c.set_websocket_handler([](const std::string& payload, malloy::websocket::writer_type writer) {
        std::cout << "received: " << payload << std::endl;

        // Response
        std::string resp = "received: " + payload;

        // Echo back
        writer(std::move(resp));
    });

    // Start
    c.start();

    return EXIT_SUCCESS;
}
