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

        // A simple GET route handler
        router.add(method::get, "/", [](const auto& req) {
            response resp{ status::ok };
            resp.body() = "<html><body><h1>Hello Malloy!</h1><p>Demo: server-routing-regex</p></body></html>";

            return resp;
        });

        // A regex route without capturing
        router.add(method::get, "^/regex", [](const auto& req) {
            response resp{ status::ok };
            resp.body() = "regex";

            return resp;
        });

        // A regex route with capturing
        router.add(method::get, "^/regex/\\d+$", [](const auto& req) {
            response resp{ status::ok };
            resp.body() = "^/regex/\\d+$";

            return resp;
        });

        // A regex route with one capturing group
        router.add(method::get, "^/regex/(\\w+)$", [](const auto& req, const std::vector<std::string>& captures) {
            std::string body;
            body += "^/regex/(\\w)$\n";
            body += "\n";
            body += "  captures:\n";
            for (std::size_t i = 0; i < captures.size(); i++)
                body += "    " + std::to_string(i) + ": " + captures[i] + "\n";

            response resp{ status::ok };
            resp.body() = body;

            return resp;
        });

        // A regex route with two capturing groups
        router.add(method::get, R"(^/regex\?one=(\w+)&two=(\w+)$)", [](const auto& req, const std::vector<std::string>& captures) {
            std::string body;
            body += "captures:\n";
            for (std::size_t i = 0; i < captures.size(); i++)
                body += "  " + std::to_string(i) + ": " + captures[i] + "\n";

            response resp{ status::ok };
            resp.body() = body;

            return resp;
        });
    }

    // Start
    [[maybe_unused]] auto session = start(std::move(c));

    // Keep the application alive
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    return EXIT_SUCCESS;
}
