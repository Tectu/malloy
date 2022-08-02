#include "../../../example.hpp"

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

    // Create the router
    auto& router = c.router();
    {
        using namespace malloy::http;
        using namespace malloy::server::http;

        // GET /foo
        router.add(method::get, "/foo", [](const auto& req) {
            response resp{ status::ok };
            resp.body() = "GET /foo";

            return resp;
        });

        // POST /foo
        router.add(method::post, "/foo", [](const auto& req) {
            response resp{ status::ok };
            resp.body() = "POST /foo";

            return resp;
        });

        // DELETE /foo
        router.add(method::delete_, "/foo", [](const auto& req) {
            response resp{ status::ok };
            resp.body() = "DELETE /foo";

            return resp;
        });

        // Add a preflight for the above endpoints
        preflight_config preflight_cfg;
        preflight_cfg.origin = "http://127.0.0.1:8080";
        router.add_preflight("/foo", preflight_cfg);
    }

    // Start
    [[maybe_unused]] auto session = start(std::move(c));

    // Keep the application alive
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    return EXIT_SUCCESS;
}
