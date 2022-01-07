#include "../../example.hpp"
#include "../../ws_handlers.hpp"

#include <malloy/server/controller.hpp>
#include <malloy/server/routing/router.hpp>

#include <iostream>

int main()
{
    // Create malloy controller config
    malloy::server::controller::config cfg;
    cfg.interface   = "127.0.0.1";
    cfg.port        = 8080;
    cfg.doc_root    = examples_doc_root;
    cfg.num_threads = 5;
    cfg.logger      = create_example_logger();

    // Create malloy controller
    malloy::server::controller c{cfg};

#if MALLOY_FEATURE_TLS
    // Setup TLS (SSL)
    const auto& cert_path = examples_doc_root / "malloy.cert";
    const auto& key_path  = examples_doc_root / "malloy.key";
    if (!c.init_tls(cert_path, key_path)) {
        std::cerr<< "could not initialize TLS context." << std::endl;
        return EXIT_FAILURE;
    }
#endif

    // Add some routes
    auto& router = c.router();
    { // TODO: Should the first two be done via optional capture groups?
        // Add a websocket endpoint
        router.add_websocket("/", [](const malloy::http::request<>& req, auto writer){
            malloy::examples::ws::accept_and_send(req, writer, fmt::format("echo at /: {}", req.body()));
        });

        // Add a websocket endpoint
        router.add_websocket("/echo", [](const malloy::http::request<>& req, auto writer){
            malloy::examples::ws::accept_and_send(req, writer, fmt::format("echo at /echo: {}", req.body()));
        });

        // Add a websocket endpoint
        router.add_websocket("/timer", [](const malloy::http::request<>& req, auto writer){
            std::make_shared<malloy::examples::ws::server_timer>(writer)->run(req);
        });
    }

    // Start
    start(std::move(c)).run();

    return EXIT_SUCCESS;
}
