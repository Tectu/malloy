#include "../../example_logger.hpp"

#include <malloy/core/http/generator.hpp>
#include <malloy/core/http/session/manager.hpp>
#include <malloy/core/http/session/session.hpp>
#include <malloy/core/http/session/storage_memory.hpp>
#include <malloy/server/routing/router.hpp>
#include <malloy/server/controller.hpp>

#include <iostream>

int main()
{
    using namespace std::chrono_literals;

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

    // Create the session manager
    auto session_storage = std::make_shared<malloy::http::sessions::storage_memory>();
    auto session_manager = std::make_shared<malloy::http::sessions::manager>(session_storage);

    // Create the router
    auto router = c.router();
    if (router) {
        using namespace malloy;
        using namespace malloy::http;

        // Use a session to increment a counter
        router->add(method::get, "/", [session_manager](const auto &req)
        {
            response res{status::ok};

            // Get the session
            auto ses = session_manager->start(req, res);
            if (!ses)
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

        // Logout
        router->add(method::get, "/logout", [session_manager](const auto& req)
        {
            response resp{ status::ok };

            session_manager->destroy(req, resp);

            return resp;
        });

        // Clear expired sessions
        router->add(method::get, "/clear_expired", [session_manager](const auto& req)
        {
            const std::size_t count = session_manager->destroy_expired(10s);

            response resp{ status::ok };
            resp.body() = "Expired sessions: " + std::to_string(count);

            return resp;
        });
    }

    // Start
    c.start();

    // Keep the application alive
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    return EXIT_SUCCESS;
}
