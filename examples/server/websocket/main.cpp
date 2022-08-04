#include "../../example.hpp"
#include "../../ws_handlers.hpp"

#include <malloy/server/routing_context.hpp>
#include <malloy/server/routing/router.hpp>

#include <iostream>

int main()
{
    // Create malloy controller config
    malloy::server::routing_context::config cfg;
    setup_example_config(cfg);
    cfg.interface   = "127.0.0.1";
    cfg.port        = 8080;

    // Create malloy controller
    malloy::server::routing_context c{cfg};

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
    {
        // Add a websocket endpoint
        // This will simply echo back everything the client sends until the client closes the connection
        router.add_websocket("/echo", [](const malloy::http::request<>& req, auto connection) {
            std::make_shared<malloy::examples::ws::ws_echo<false>>(connection)->run(req);
        });

        // Add a websocket endpoint
        // This will send a couple of messages to the client and then close the connection
        router.add_websocket("/timer", [](const malloy::http::request<>& req, auto connection) {
            std::make_shared<malloy::examples::ws::server_timer>(connection)->run(req);
        });
    }

    // Start
    start(std::move(c)).run();

    return EXIT_SUCCESS;
}
