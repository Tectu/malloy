#pragma once

#include <malloy/server/app_fw/app.hpp>

class database;
class page_master;

/**
 * Top-level application.
 */
class app :
    public malloy::server::app_fw::app
{
public:
    app(
        std::shared_ptr<spdlog::logger> logger,
        malloy::server::app_fw::app::environment env,
        std::shared_ptr<database> db
    );

    ~app() override = default;

    [[nodiscard]]
    bool init() override;

private:
    std::shared_ptr<database> m_db;
    std::shared_ptr<page_master> m_master_page;
};
