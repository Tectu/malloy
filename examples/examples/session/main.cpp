#include "malloy/controller.hpp"
#include "malloy/http/generator.hpp"
#include "malloy/http/router.hpp"
#include "malloy/http/session/manager.hpp"
#include "malloy/http/session/session.hpp"
#include "malloy/http/session/storage_memory.hpp"

#include <iostream>

int main()
{
    using namespace std::chrono_literals;

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

    // Create the session manager
    auto session_manager = std::make_shared<malloy::http::sessions::manager>();
    auto session_storage = std::make_shared<malloy::http::sessions::storage_memory>();
    if (not session_manager->init(session_storage, 24h)) {
        std::cerr << "could not initialize session manager." << std::endl;
    }

    // Create the router
    auto router = c.router();
    if (router) {
        using namespace malloy::http;

        // Use a session to increment a counter
        router->add(method::get, "/", [session_manager](const auto &req)
        {
            response res{status::ok};

            // Get the session
            auto ses = session_manager->start_session(req, res);
            if (not ses)
                return generator::server_error("session management error.");

            // Get counter value
            int counter;
            counter = std::stoi(ses->get("counter").value_or("0"));

            // Store the new counter value
            ses->set("counter", std::to_string(counter+1));

            // Assemble body
            res.body() = "Counter: " + std::to_string(counter);

            return res;
        });
    }

    // Start
    c.start();

    return EXIT_SUCCESS;
}
