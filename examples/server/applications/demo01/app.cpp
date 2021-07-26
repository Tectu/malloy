#include "app.hpp"
#include "page_master.hpp"
#include "apps/gallery/app.hpp"

#include <malloy/server/routing/router.hpp>
#include <spdlog/logger.h>

app::app(
    std::shared_ptr<spdlog::logger> logger,
    malloy::server::app_fw::app::environment env,
    std::shared_ptr<database> db
) :
    malloy::server::app_fw::app(
        std::move(logger),
        "top-level",
        std::move(env)
    ),
    m_db(std::move(db))
{
    // Sanity check
    if (!m_db)
        throw std::invalid_argument("no valid database provided.");
}

bool
app::init()
{
    // Create master page
    m_master_page = std::make_shared<page_master>();

    // Create sub-apps
    make_subapp<apps::gallery::app>("gallery", m_db, m_master_page);

    // Endpoints
    m_logger->warn("FOO: {}", m_env.app.assets_fs_path.string());
    m_router->add_file_serving("/assets", m_env.app.assets_fs_path);

    return true;
}
