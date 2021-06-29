#include "../../example_logger.hpp"
#include "../../ws_handlers.hpp"

            
#include <malloy/websocket/connection.hpp>
#include <malloy/server/controller.hpp>
#include <malloy/server/routing/router.hpp>

#include <spdlog/fmt/fmt.h>

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
    cfg.logger      = create_example_logger();

    // Create malloy controller
    malloy::server::controller c;
    if (!c.init(cfg)) {
        std::cerr << "could not start controller." << std::endl;
        return EXIT_FAILURE;
    }

    // Add some routes
    auto router = c.router();
    if (router) { // TODO: Should the first two be done via optional capture groups?
        // Add a websocket endpoint
        router->add_websocket("/", [](const malloy::http::request<>& req, auto writer){
            malloy::examples::ws::accept_and_send(req, writer, fmt::format("echo at /: {}", req.body()));
        });

        // Add a websocket endpoint
        router->add_websocket("/echo", [](const malloy::http::request<>& req, auto writer){
            malloy::examples::ws::accept_and_send(req, writer, fmt::format("echo at /echo: {}", req.body()));
        });

        // Add a websocket endpoint
        router->add_websocket("/timer", [](const malloy::http::request<>& req, auto writer){
            std::make_shared<malloy::examples::ws::server_timer>(writer)->run(req);

        });
    }

    // Start
    c.start();

    // Keep the application alive
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    return EXIT_SUCCESS;
}
