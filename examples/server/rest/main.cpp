#include "../../example.hpp"
#include "../../ws_handlers.hpp"

#include <malloy/core/http/generator.hpp>
#include <malloy/server/routing_context.hpp>
#include <malloy/server/routing/router.hpp>
#include <malloy/server/rest/rest.hpp>

#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>

struct employee
{
    using id_type = std::size_t;

    malloy::rest::field<id_type, "id"> id{ std::numeric_limits<id_type>::max() };
    malloy::rest::field<std::string, "username"> username;
    malloy::rest::field<std::string, "firstname", malloy::rest::field_flags::optional> firstname;
    malloy::rest::field<std::string, "lastname", malloy::rest::field_flags::optional> lastname;

    [[nodiscard]]
    nlohmann::json
    to_json() const
    {
        nlohmann::json j;

        field_to_json(j, id);
        field_to_json(j, username);
        field_to_json(j, firstname);
        field_to_json(j, lastname);

        return j;
    }

    bool
    from_json(nlohmann::json&& j)
    {
        field_from_json(j, id);
        field_from_json(j, username);
        field_from_json(j, firstname);
        field_from_json(j, lastname);

        return true;
    }

    [[nodiscard]]
    std::string
    dump()
    {
        std::stringstream ss;

        ss << "id       : " << id.value_or(0) << "\n";
        ss << "username : " << username.value_or("") << "\n";
        ss << "firstname: " << firstname.value_or("") << "\n";
        ss << "lastname : " << lastname.value_or("") << "\n";

        return ss.str();
    }
};

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

    // Employees "database"
    malloy::server::rest::store::in_memory<employee> db;

    auto logger = cfg.logger;

    {
        using namespace malloy;

        c.router().add_rest(
            server::rest::make_resource<employee>(
                "employees",

                // GET (all)
                [logger, &db](const std::size_t limit, const std::size_t offset) -> rest::response_handle {
                    auto es = db.get(limit, offset);

                    return rest::success_collection{ std::move(es) };
                },

                // GET
                [logger, &db](const std::size_t id) -> rest::response_handle {
                    auto e = db.get(id);
                    if (!e)
                        return rest::success{ };   // ToDo

                    return rest::success<employee>{ *e };
                },

                // POST
                [logger, &db]() -> rest::response_handle {
                    auto e = db.add();

                    return rest::created{ e };
                },

                // DELETE
                [logger, &db](const std::size_t id) -> rest::response_handle {
                    const bool success = db.remove(id);
                    if (!success)
                        return rest::success{ };   // ToDo

                    return rest::success{ };
                },

                // PATCH
                [logger, &db](const std::size_t id, employee&& obj) -> rest::response_handle {
                    auto e = db.modify(id, std::move(obj));
                    if (!e)
                        return rest::success{ };    // ToDo

                    return rest::success{ *e };
                }
            )
        );
    }

    // Start
    start(std::move(c)).run();

    return EXIT_SUCCESS;
}
