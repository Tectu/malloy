#include <malloy/core/html/form.hpp>
#include <malloy/core/html/form_renderer.hpp>
#include <malloy/server/app_fw/page.hpp>

#include <sstream>

namespace apps::gallery::pages
{

    class upload:
        public malloy::server::app_fw::page
    {
    public:
        std::shared_ptr<malloy::html::form> m_form;

        upload()
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
        std::string
        render() const override
        {
            std::ostringstream ss;
            malloy::html::form_renderer_basic fr;

            ss << "<html>\n";
            ss << "  <body>\n";
            ss << "    <h1>Gallery - Upload</h1>\n";
            ss << "    <p>Let's upload an image!</p>\n";
            ss << fr.render(*m_form);
            ss << "  </body>\n";
            ss << "</html>\n";

            return ss.str();
        }
    };

}
