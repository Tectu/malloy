#include "database.hpp"
#include "apps/gallery/app.hpp"
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
    server::app_fw::app::environment env {
        .site {
            .base_url = "http://127.0.0.1:8080",
        },
        .app {
            .base_url = "http://127.0.0.1:8080",
        }
    };

    // Create top-level applications
    auto app_gallery = std::make_shared<apps::gallery::app>(
        cfg.logger->clone("apps | gallery"),
        env,
        db
    );

    // Initialize router
    {
        auto router = c.router();

        // Add top-level apps
        router->add_subrouter("/gallery", app_gallery->router());
    }

    // Start
    c.start();

    // Keep the application alive
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    return EXIT_SUCCESS;
}
