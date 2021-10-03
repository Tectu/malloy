#include "app.hpp"
#include "database.hpp"
#include "../../../example.hpp"

#include <malloy/server/controller.hpp>
#include <malloy/server/routing/router.hpp>

#include <iostream>

int main()
{
    using namespace malloy;

    // Create malloy controller config
    server::controller::config cfg;
    cfg.interface   = "127.0.0.1";
    cfg.port        = 8080;
    cfg.doc_root    = examples_doc_root;
    cfg.num_threads = 1;
    cfg.logger      = create_example_logger();

    // Create malloy controller
    server::controller c;
    if (!c.init(cfg)) {
        std::cerr << "could not start controller." << std::endl;
        return EXIT_FAILURE;
    }

    // Setup the database
    auto db = std::make_shared<database>(cfg.logger->clone("database"));
    if (!db->init()) {
        cfg.logger->critical("could not initialize database.");
        return EXIT_FAILURE;
    }

    // Setup application environment
    server::app_fw::environment env {
        .site {
            .base_url       = "http://127.0.0.1:8080",
        },
        .app {
            .base_url       = "http://127.0.0.1:8080",
            .assets_fs_path = "../../examples/server/applications/demo01/assets",
        }
    };

    // Create top-level application
    auto toplevel_app = std::make_shared<app>(
        cfg.logger->clone("app"),
        env,
        db
    );
    if (!toplevel_app->init()) {
        cfg.logger->critical("initializing top-level app failed.");
        return EXIT_FAILURE;
    }

    // Initialize router
    {
        auto router = c.router();

        // Add top-level app
        router->add_subrouter("/apps", toplevel_app->router());
    }

    // Start
    c.start();

    // Keep the application alive
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    return EXIT_SUCCESS;
}
