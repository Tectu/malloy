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

    std::string name;

    [[nodiscard]]
    nlohmann::json
    to_json() const
    {
        nlohmann::json j;

        j["name"] = name;

        return j;
    }

    bool
    from_json(nlohmann::json&& j)
    {
        name = j["name"];

        return true;
    }

    [[nodiscard]]
    std::string
    dump()
    {
        std::stringstream ss;

        ss << "name: " << name << "\n";

        return ss.str();
    }
};

std::vector<employee> employees;

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
        using namespace malloy;

        c.router().add_rest(
            server::rest::make_resource<employee>(
                "employees",

                // Get all
                [logger](const std::size_t limit, const std::size_t offset) {
                    logger->warn("GetAll {} {}", limit, offset);
                    return rest::success{ };
                },

                // Get
                [logger](const std::size_t id) {
                    logger->warn("GET {}", id);

                    employee e;

                    return rest::success{ e };
                },

                // Create
                [logger]() {
                    logger->warn("POST");

                    return rest::success{ };
                },

                // Remove
                [logger](const std::size_t id) {
                    logger->warn("DELETE {}", id);

                    return rest::success{ };
                },

                // Modify
                [logger](const std::size_t id, employee&& obj) {
                    logger->warn("PATCH {}", id);

                    logger->warn("\n{}\n", obj.dump());

                    return rest::success{ };
                }
            )
        );
    }

    // Start
    start(std::move(c)).run();

    return EXIT_SUCCESS;
}
