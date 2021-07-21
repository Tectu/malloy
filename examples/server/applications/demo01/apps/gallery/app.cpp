#include "app.hpp"
#include "page_overview.hpp"
#include "page_upload.hpp"

#include <malloy/server/routing/router.hpp>

using namespace apps::gallery;

app::app(std::shared_ptr<spdlog::logger> logger) :
    malloy::server::application(std::move(logger), "gallery")
{
    using namespace malloy::http;

    // Setup pages
    {
        m_page_overview = std::make_shared<pages::overview>();
        m_page_upload = std::make_shared<pages::upload>();
    }

    // Setup router
    {
        // Root page
        m_router->add_page("", m_page_overview);

        // Upload page
        m_router->add_page("/upload", m_page_upload);

        // Upload POST endpoint
        m_router->add(method::post, "/upload", [this](const auto& req){
            auto form = *m_page_upload->m_form;

            // Parse form
            if (!form.parse(req))
                return generator::bad_request("invalid form data.");

            // Upload was successful, redirect to root page
            return generator::redirect(status::see_other, "");
        });
    }
}
