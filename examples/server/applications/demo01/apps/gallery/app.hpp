#pragma once

#include <malloy/server/application.hpp>

class database;

namespace apps::gallery
{
    namespace pages
    {
        class overview;
        class upload;
    }

    class app :
        public malloy::server::application
    {
    public:
        app(
            std::shared_ptr<spdlog::logger> logger,
            std::shared_ptr<database> db
        );

        ~app() noexcept override = default;

    private:
        std::shared_ptr<database> m_db;
        std::shared_ptr<pages::overview> m_page_overview;
        std::shared_ptr<pages::upload> m_page_upload;
    };
}
