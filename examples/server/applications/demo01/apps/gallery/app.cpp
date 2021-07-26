#include "app.hpp"
#include "page_overview.hpp"
#include "page_upload.hpp"
#include "../../database.hpp"

#include <malloy/server/routing/router.hpp>

using namespace apps::gallery;

app::app(
    std::shared_ptr<database> db,
    std::shared_ptr<malloy::server::app_fw::page_master> master_page
) :
    m_db(std::move(db)),
    m_master_page(std::move(master_page))
{
    // Sanity check database
    if (!m_db)
        throw std::invalid_argument("no valid database provided.");
}

bool
app::init()
{
    using namespace malloy::http;

    // Setup pages
    {
        m_page_overview = std::make_shared<pages::overview>(m_master_page);
        m_page_upload = std::make_shared<pages::upload>(m_master_page);
    }

    // Setup router
    {
        // Root page
        add_page("", m_page_overview);

        // Upload page
        add_page("/upload", m_page_upload);

        // Upload POST endpoint
        m_router->add(method::post, "/upload", [this](const auto& req){
            auto form = *m_page_upload->m_form;

            // Parse form
            if (!form.parse(req))
                return generator::bad_request("invalid form data.");

            // Re-populate the form's pre-filled values
            form.populate_values_from_parsed_data();

            // Extract values & perform sanity checks
            const std::string& caption = form.content("caption").value_or("");
            const std::string& image = form.content("image").value_or("");
            {
                if (caption.empty())
                    return generator::bad_request("caption must not be empty.");

                if (image.empty())
                    return generator::bad_request("image must not be empty.");
            }

            // Insert into database
            if (const bool successful = m_db->add_image(caption, image); !successful) {
                m_logger->warn("could not insert image into database.");

                return generator::server_error("could not store image in database.");
            }

            // Upload was successful, redirect to root page
            return generator::redirect(status::see_other, "");
        });
    }

    return true;
}
