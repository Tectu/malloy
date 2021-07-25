#include "app.hpp"
#include "page_master.hpp"
#include "apps/gallery/app.hpp"

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

    // Create master page
    m_master_page = std::make_shared<page_master>();

    // Create sub-apps
    {
        // Gallery
        add_subapp(std::make_shared<apps::gallery::app>(
            m_logger->clone("gallery"),
            env.make_sub_environment("gallery"),
            m_db,
            m_master_page
        ));
    }
}
