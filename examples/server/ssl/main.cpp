#include "../../example.hpp"

#include <malloy/core/http/generator.hpp>
#include <malloy/server/controller.hpp>
#include <malloy/server/routing/router.hpp>

#include <iostream>

int main()
{
    // Create malloy controller config
    malloy::server::routing_context::config cfg;
    cfg.interface   = "127.0.0.1";
    cfg.port        = 8080;
    cfg.doc_root    = examples_doc_root;
    cfg.num_threads = 5;
    cfg.logger      = create_example_logger();

    // Create malloy controller
    malloy::server::routing_context c{cfg};

    // Setup TLS (SSL)
    const auto& cert_path = examples_doc_root / "malloy.cert";
    const auto& key_path  = examples_doc_root / "malloy.key";
    if (!c.init_tls(cert_path, key_path)) {
        std::cerr<< "could not initialize TLS context." << std::endl;
        return EXIT_FAILURE;
    }

    // Create the router
    auto& router = c.router();
    {
        using namespace malloy::http;

        // A simple GET route handler
        router.add(method::get, "/", [](const auto& req) {
            response res{status::ok};
            res.body() = "<html><body><h1>Hello World!</h1><p>This should work with HTTPS!</p></body></html>";
            return res;
        });
    }

    // Start
    start(std::move(c)).run();

    return EXIT_SUCCESS;
}
