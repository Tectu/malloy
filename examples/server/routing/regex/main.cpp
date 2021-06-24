#include "../../../example_logger.hpp"

#include <malloy/server/controller.hpp>
#include <malloy/http/generator.hpp>
#include <malloy/server/routing/router.hpp>

#include <iostream>
#include <memory>

int main()
{
    const std::filesystem::path doc_root = "../../../../../examples/server/static_content";

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

    // Create the router
    auto router = c.router();
    if (router) {
        using namespace malloy::http;

        // A simple GET route handler
        router->add(method::get, "/", [](const auto& req) {
            response resp{ status::ok };
            resp.body() = "<html><body><h1>Hello Malloy!</h1><p>Demo: server-routing-regex</p></body></html>";

            return resp;
        });

        // A regex route without capturing
        router->add(method::get, "^/regex", [](const auto& req) {
            response resp{ status::ok };
            resp.body() = "regex";

            return resp;
        });

        // A regex route with capturing
        router->add(method::get, "^/regex/\\d+$", [](const auto& req) {
            response resp{ status::ok };
            resp.body() = "^/regex/\\d+$";

            return resp;
        });

        // A regex route with capturing
        router->add(method::get, "^/regex/(\\w+)$", [](const auto& req, std::vector<std::string> captures) {
            response resp{ status::ok };
            resp.body() = "/regex/" + captures.at(0);

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