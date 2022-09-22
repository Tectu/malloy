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

    id_type id = std::numeric_limits<id_type>::max();
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

struct employees
{
    [[nodiscard]]
    employee
    add()
    {
        std::lock_guard lock(m_mtx);

        employee e;
        e.id = generate_id();

        m_employees.emplace_back(e);

        return e;
    }

    [[nodiscard]]
    std::optional<employee>
    get(const std::size_t id)
    {
        std::lock_guard lock(m_mtx);

        const auto& it = std::find_if(
            std::cbegin(m_employees),
            std::cend(m_employees),
            [id](const auto& e) {
                return e.id == id;
            }
        );
        if (it == std::cend(m_employees))
            return { };

        return *it;
    }

    [[nodiscard]]
    std::vector<employee>
    get(const std::size_t limit, const std::size_t offset)
    {
        std::vector<employee> ret;
        ret.reserve(limit);

        for (std::size_t i = 0; i < limit; i++) {
            const std::size_t index = i + offset;
            if (index >= m_employees.size())
                break;

            ret.emplace_back(m_employees[index]);
        }

        return ret;
    }

    [[nodiscard]]
    bool
    remove(const std::size_t id)
    {
        std::lock_guard lock(m_mtx);

        return (
            std::erase_if(
                m_employees,
                [id](const auto& e) {
                    return e.id == id;
                }
            )
        ) > 0;
    }

private:
    std::vector<employee> m_employees;
    std::mutex m_mtx;
    std::size_t m_id_counter = 0;

    [[nodiscard]]
    std::size_t
    generate_id()
    {
        return m_id_counter++;
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
    employees db;

    auto logger = cfg.logger;

    {
        using namespace malloy;

        c.router().add_rest(
            server::rest::make_resource<employee>(
                "employees",

                // GET (all)
                [logger, &db](const std::size_t limit, const std::size_t offset) {
                    logger->warn("GetAll {} {}", limit, offset);

                    auto es = db.get(limit, offset);

                    return rest::success_collection{ std::move(es) };
                },

                // GET
                [logger, &db](const std::size_t id) -> rest::response {
                    logger->warn("GET {}", id);

                    auto e = db.get(id);
                    if (!e)
                        return rest::success{ };   // ToDo

                    return rest::success{ *e };
                },

                // POST
                [logger, &db]() {
                    logger->warn("POST");

                    auto e = db.add();

                    return rest::created{ e };
                },

                // DELETE
                [logger, &db](const std::size_t id) {
                    logger->warn("DELETE {}", id);

                    const bool success = db.remove(id);
                    if (!success)
                        return rest::success{ };   // ToDo

                    return rest::success{ };
                },

                // PATCH
                [logger, &db](const std::size_t id, employee&& obj) {
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
