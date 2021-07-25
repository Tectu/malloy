#pragma once

#include <malloy/server/app_fw/app.hpp>

class database;

namespace apps::gallery
{
    namespace pages
    {
        class overview;
        class upload;
    }

    class app :
        public malloy::server::app_fw::app
    {
    public:
        app(
            std::shared_ptr<spdlog::logger> logger,
            malloy::server::app_fw::app::environment env,
            std::shared_ptr<database> db
        );

        ~app() noexcept override = default;

    private:
        std::shared_ptr<database> m_db;
        std::shared_ptr<pages::overview> m_page_overview;
        std::shared_ptr<pages::upload> m_page_upload;
    };
}
