#include <malloy/core/html/form.hpp>
#include <malloy/core/html/form_renderer.hpp>
#include <malloy/server/app_fw/page_content.hpp>

#include <sstream>

namespace apps::gallery::pages
{

    class upload:
        public malloy::server::app_fw::page_content
    {
    public:
        std::shared_ptr<malloy::html::form> m_form;

        explicit
        upload(
            std::shared_ptr<malloy::server::app_fw::page_master> master_page
        ) :
            malloy::server::app_fw::page_content(
                "assets/templates/upload.html",
                std::move(master_page)
            )
        {
            // Setup form
            {
                // Create form
                m_form = std::make_shared<malloy::html::form>(
                    malloy::http::method::post,
                    "http://127.0.0.1:8080/gallery/upload",
                    malloy::html::form::encoding::multipart
                );

                // Caption field
                m_form->add_field({
                    .name = "caption",
                    .type = "text",
                    .label = "Caption:",
                    .required = true,
                });

                // Image field
                m_form->add_field({
                    .name = "image",
                    .type = "file",
                    .label = "Image:",
                    .required = true,
                });

                // Submit button
                m_form->add_field({
                    .name = "Upload!",
                    .type = "submit"
                });
            }

        }

        [[nodiscard]]
        nlohmann::json
        data() const override
        {
            malloy::html::form_renderer_basic fr;
            nlohmann::json j;

            j["form"] = fr.render(*m_form);

            return j;
        }

    };

}
