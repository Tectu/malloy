#include "../../example.hpp"

#include <malloy/core/http/generator.hpp>
#include <malloy/core/http/session/manager.hpp>
#include <malloy/core/http/session/session.hpp>
#include <malloy/core/http/session/storage_memory.hpp>
#include <malloy/server/routing/router.hpp>
#include <malloy/server/routing_context.hpp>

#include <iostream>

int main()
{
    using namespace std::chrono_literals;

    // Create malloy controller config
    malloy::server::routing_context::config cfg;
    cfg.interface   = "127.0.0.1";
    cfg.port        = 8080;
    cfg.doc_root    = examples_doc_root;
    cfg.num_threads = 5;
    cfg.logger      = create_example_logger();

    // Create malloy controller
    malloy::server::routing_context c{cfg};

    // Create the session manager
    auto session_storage = std::make_shared<malloy::http::sessions::storage_memory>();
    auto session_manager = std::make_shared<malloy::http::sessions::manager>(session_storage);

    // Create the router
    auto& router = c.router();
    {
        using namespace malloy;
        using namespace malloy::http;

        // Use a session to increment a counter
        router.add(method::get, "/", [session_manager](const auto &req)
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

        // Check validity
        router.add(method::get, "/check", [session_manager](const auto& req)
        {
            response resp{ status::ok };
            resp.body() = "valid session: " + std::string{(session_manager->is_valid(req) ? "true" : "false")};

            return resp;
        });

        // Logout
        router.add(method::get, "/logout", [session_manager](const auto& req)
        {
            response resp{ status::ok };

            session_manager->destroy(req, resp);

            return resp;
        });

        // Clear expired sessions
        router.add(method::get, "/clear_expired", [session_manager](const auto& req)
        {
            const std::size_t count = session_manager->destroy_expired(10s);

            response resp{ status::ok };
            resp.body() = "Expired sessions: " + std::to_string(count);

            return resp;
        });
    }

    // Start
    start(std::move(c)).run(); // Keep the application alive
    return EXIT_SUCCESS;
 }
