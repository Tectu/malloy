#include "malloy/controller.hpp"
#include "malloy/http/generator.hpp"
#include "malloy/http/routing/router.hpp"

#include <iostream>
#include <memory>

int main()
{
    const std::filesystem::path doc_root = "../../../../examples/static_content";

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

    // Enable termination signals
    c.enable_termination_signals();

    // Create the router
    auto router = c.router();
    if (router) {
        using namespace malloy::http;

        // A simple GET route handler
        router->add(method::get, "/", [](const auto &req)
        {
            response res{status::ok};
            res.body() = "<html><body><h1>Hello World!</h1><p>from malloy</p></body></html>";
            return res;
        });
    }

    // Start
    c.start();

    return EXIT_SUCCESS;
}
