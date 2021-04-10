#include "malloy/controller.hpp"
#include "malloy/http/generator.hpp"
#include "malloy/http/router.hpp"

#include <iostream>
#include <memory>

int main()
{
    const std::filesystem::path doc_root = "../../../../examples/static_content";

    // Create the router
    auto router = std::make_shared<malloy::http::server::router>();
    {
        using namespace malloy::http;

        // A simple GET route handler
        router->add(method::get, "/", [](const auto& req) {
            response res{status::ok};
            res.body() = "<html><body><h1>Hello World!</h1><p>some content...</p></body></html>";
            return res;
        });

        // Add a route to an existing file
        router->add(method::get, "/file", [doc_root](const auto& req) {
            return generator::file(doc_root, "index.html");
        });

        // Add a route to a non-existing file
        router->add(method::get, "/file_nonexist", [doc_root](const auto& req) {
            return generator::file(doc_root, "/some_nonexisting_file.xzy");
        });

        // Add some redirections
        router->add_redirect(status::permanent_redirect, "/redirect1", "/");
        router->add_redirect(status::temporary_redirect, "/redirect2", "/");

        // Add some file serving
        router->add_file_serving("/files", doc_root);
    }

    // Create malloy controller config
    malloy::server::controller::config cfg;
    cfg.interface   = "127.0.0.1";
    cfg.port        = 8080;
    cfg.doc_root    = doc_root;
    cfg.num_threads = 5;
    cfg.router      = router;

    // Create malloy controller
    malloy::server::controller c;
    if (not c.init(cfg)) {
        std::cerr << "could not start controller." << std::endl;
        return EXIT_FAILURE;
    }

    // Start
    c.start();

    return EXIT_SUCCESS;
}
