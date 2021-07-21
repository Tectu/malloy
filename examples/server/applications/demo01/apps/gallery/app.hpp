#pragma once

#include <malloy/server/application.hpp>

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
        explicit
        app(std::shared_ptr<spdlog::logger> logger);

        ~app() noexcept override = default;

    private:
        std::shared_ptr<pages::overview> m_page_overview;
        std::shared_ptr<pages::upload> m_page_upload;
    };
}
