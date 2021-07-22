#pragma once

#include <memory>

struct sqlite3;

namespace spdlog
{
    class logger;
}

/**
 * Database class.
 *
 * This is a VERY basic implementation. It was kept to a bare minimum for demonstration purposes.
 * It doesn't do any SQL injection protections/checks or other very important security checks.
 */
class database
{
public:
    explicit database(std::shared_ptr<spdlog::logger> logger);
    database(const database&) = delete;
    database(database&&) noexcept = delete;
    virtual ~database() noexcept;

    database& operator=(const database&) = delete;
    database& operator=(database&&) noexcept = delete;

    bool
    init();

    bool
    add_image(const std::string& caption, const std::string& data);

private:
    std::shared_ptr<spdlog::logger> m_logger;
    sqlite3* m_db = nullptr;
};
