#include "database.hpp"
#include "3rdparty/sqlite/sqlite3.h"

#include <spdlog/logger.h>

#include <stdexcept>

database::database(std::shared_ptr<spdlog::logger> logger) :
    m_logger(std::move(logger))
{
    if (!m_logger)
        throw std::invalid_argument("no valid logger provided.");
}

database::~database() noexcept
{
    sqlite3_close(m_db);
}

bool
database::init()
{
    // Open DB
    if (const int rc = sqlite3_open("demo_01.sqlite", &m_db); rc != SQLITE_OK) {
        m_logger->critical("could not open database: {}", sqlite3_errmsg(m_db));
        sqlite3_close(m_db);
        m_db = nullptr;

        return false;
    }

    // Create tables
    {
        std::vector<std::string> statements;

        // Table "images"
        statements.emplace_back(
            "CREATE TABLE IF NOT EXISTS images"
            "("
            "  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
            "  caption TEXT NOT NULL,"
            "  data BLOB NOT NULL"
            ");"
        );

        for (const auto& stmt : statements) {
            if (const int rc = sqlite3_exec(m_db, stmt.c_str(), nullptr, nullptr, nullptr); rc != SQLITE_OK) {
                m_logger->critical("could not create table: {}", sqlite3_errmsg(m_db));

                return false;
            }
        }
    }

    return true;
}

bool
database::add_image(const std::string& caption, const std::string& data)
{
    const std::string sql{
        "INSERT INTO images\n"
        "(caption, data)\n"
        "VALUES('" + caption + "', '" + data + "');"
    };

    if (const int rc = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, nullptr); rc != SQLITE_OK) {
        m_logger->error("could not insert image into database: {}", sqlite3_errmsg(m_db));
        return false;
    }

    return true;
}
