#include "../../form_renderer.hpp"

#include <malloy/core/html/form.hpp>
#include <malloy/core/html/form_renderer.hpp>
#include <malloy/server/app_fw/page_content.hpp>

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
                "../../examples/server/applications/demo01/apps/gallery/assets/templates/upload.html",
                std::move(master_page)
            )
        {
            // Setup form
            {
                // Create form
                m_form = std::make_shared<malloy::html::form>(
                    malloy::http::method::post,
                    "http://127.0.0.1:8080/apps/gallery/upload",
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
            nlohmann::json j;

            j["form"] = m_form_renderer.render(*m_form);

            return j;
        }

    private:
        form_renderer m_form_renderer;
    };

}
