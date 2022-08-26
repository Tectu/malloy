#include "../../example.hpp"
#include "../../ws_handlers.hpp"

#include <malloy/core/http/generator.hpp>
#include <malloy/server/routing_context.hpp>
#include <malloy/server/routing/router.hpp>
#include <malloy/server/rest/rest.hpp>

#include <iostream>
#include <memory>

struct employee
{
    using id_type = std::size_t;

    [[nodiscard]]
    nlohmann::json
    to_json() const
    {
        return { };
    }
};

int main()
{
    // Create malloy controller config
    malloy::server::routing_context::config cfg;
    setup_example_config(cfg);
    cfg.interface   = "127.0.0.1";
    cfg.port        = 8080;

    // Create malloy controller
    malloy::server::routing_context c{cfg};

    auto logger = cfg.logger;

    {
        using namespace malloy::server;


        auto r = rest::make_resource<employee>(
            "employees",

            // Get all
            [logger](const std::size_t limit, const std::size_t offset) {
                logger->warn("GetAll {} {}", limit, offset);
                return rest::success{ };
            },

            // Get
            [logger](const std::size_t id) {
                logger->warn("GET {}", id);
                return rest::success{ };
            },

            // Create
            [](const std::size_t id, employee&& obj) {
                return rest::success{ };
            },

            // Remove
            [](const std::size_t id, employee&& obj) {
                return rest::success{ };
            },

            // Modify
            [](const std::size_t id, employee&& obj) {
                return rest::success{ };
            }
        );


        c.router().add_rest(std::move(r));
    }

    // Start
    start(std::move(c)).run();

    return EXIT_SUCCESS;
}
