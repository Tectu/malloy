#include "../../example.hpp"
#include "../../ws_handlers.hpp"

#include <malloy/core/http/generator.hpp>
#include <malloy/server/routing_context.hpp>
#include <malloy/server/routing/router.hpp>
#include <malloy/server/rest/rest.hpp>

#include <iostream>
#include <memory>
#include <unordered_map>

struct employee
{
    using id_type = std::size_t;

    id_type id;
    std::string name;

    [[nodiscard]]
    nlohmann::json
    to_json() const
    {
        nlohmann::json j;

        j["id"] = id;
        j["name"] = name;

        return j;
    }

    bool
    from_json(nlohmann::json&& j)
    {
        id = j["id"];
        name = j["name"];

        return true;
    }

    [[nodiscard]]
    std::string
    dump()
    {
        std::stringstream ss;

        ss << "id  : " << id << "\n";
        ss << "name: " << name << "\n";

        return ss.str();
    }
};

// Note: As the HTTP request handlers are asynchronous and we support multiple I/O threads at once, access to this
//       collection would need to be synchronized. We refrain from this here for the sake of simplicity.
std::unordered_map<std::size_t, employee> employees;     // <id, employee>

[[nodiscard]]
static
std::size_t
generate_id()
{
    static std::size_t i = 0;

    return i++;
}

int
main()
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

                // GET (all)
                [logger](const std::size_t limit, const std::size_t offset) {
                    logger->warn("GetAll {} {}", limit, offset);
                    return rest::success{ };
                },

                // GET
                [logger](const std::size_t id) {
                    logger->warn("GET {}", id);

                    employee e;

                    return rest::success{ e };
                },

                // POST
                [logger]() {
                    logger->warn("POST");

                    const auto id = generate_id();

                    employee e;

                    employees.try_emplace(id, e);

                    return rest::created{ e };
                },

                // DELETE
                [logger](const std::size_t id) {
                    logger->warn("DELETE {}", id);

                    return rest::success{ };
                },

                // PATCH
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
