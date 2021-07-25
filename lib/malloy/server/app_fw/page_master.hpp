#pragma once

#include "page.hpp"
#include "../../core/http/response.hpp"

#include <nlohmann/json.hpp>

#include <filesystem>

namespace malloy::server::app_fw
{
    class page_content;

    /**
     * Class for rendering template based HTML pages.
     *
     * @note This uses the pantor/inja template engine. See the corresponding docs for capabilities.
     */
    class page_master :
        public page
    {
    public:
        explicit
        page_master(std::filesystem::path template_path);

        page_master() = delete;
        ~page_master() override = default;

        /**
         * Renders the template without any content.
         *
         * @return The HTTP response.
         */
        [[nodiscard]]
        malloy::http::response<>
        render() const override;

        /**
         * Renders a template with content.
         *
         * @param content The content.
         * @return The HTTP response.
         */
        [[nodiscard]]
        malloy::http::response<>
        render(const page_content& content) const;

    private:
        std::filesystem::path m_tmpl_path;

        [[nodiscard]]
        virtual
        nlohmann::json
        data() const = 0;

        [[nodiscard]]
        malloy::http::response<>
        render_impl(const nlohmann::json& data, const page_content* content) const;
    };

}
