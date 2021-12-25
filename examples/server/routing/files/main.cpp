#include "../../../example.hpp"

#include <malloy/server/controller.hpp>
#include <malloy/server/listener.hpp>
#include <malloy/server/routing/router.hpp>

#include <iostream>

int main(int argc, char* argv[])
{
    // Create malloy controller config
    malloy::server::controller::config cfg;
    cfg.interface   = "127.0.0.1";
    cfg.port        = 8080;
    cfg.doc_root    = examples_doc_root;
    cfg.num_threads = 5;
    cfg.logger      = create_example_logger();

    // Create malloy controller
    malloy::server::controller c;
    if (!c.init(cfg)) {
        std::cerr << "could not start controller." << std::endl;
        return EXIT_FAILURE;
    }

    // Create top-level router
    auto router = c.router();
    if (router) {
        using namespace malloy;
        using namespace malloy::http;

        router->add_file_serving("/files", examples_doc_root);
    }

    // Start
    c.start();

    // Keep the application alive
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    return EXIT_SUCCESS;
}
