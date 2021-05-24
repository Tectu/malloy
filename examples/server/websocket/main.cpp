#include <malloy/server/controller.hpp>
#include <malloy/server/routing/router.hpp>

#include <iostream>
#include <memory>
#include <thread>

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

    // Add some routes
    auto router = c.router();
    if (router) {
        // Add a websocket endpoint
        router->add_websocket("/", [](const std::string& payload, auto writer){
            writer("echo at /: " + payload);
        });

        // Add a websocket endpoint
        router->add_websocket("/echo", [](const std::string& payload, auto writer){
            writer("echo at /echo: " + payload);
        });

        // Add a websocket endpoint
        router->add_websocket("/timer", [](const std::string& payload, auto writer){
            using namespace std::chrono_literals;

            for (std::size_t i = 0; i < 10; i++) {
                // Write to socket
                writer("i = " + std::to_string(i));

                // Sleep
                std::this_thread::sleep_for(1s);
            }
        });
    }

    // Start
    c.start();

    return EXIT_SUCCESS;
}
