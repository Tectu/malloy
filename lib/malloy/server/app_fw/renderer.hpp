#pragma once

#include <nlohmann/json.hpp>

#include <filesystem>

namespace malloy::server::app_fw
{

    class renderer
    {
    public:
        renderer() = default;
        renderer(const renderer&) = delete;
        renderer(renderer&&) noexcept = delete;
        virtual ~renderer() noexcept = default;

        renderer& operator=(const renderer&) = delete;
        renderer& operator=(renderer&&) noexcept = delete;

        /**
         * Renders a single page.
         *
         * @note The provided template path must be either absolute or relative to the working directory.
         *
         * @param tmpl_path The filesystem path to the template.
         * @param data The template data.
         * @return The rendered template.
         */
        [[nodiscard]]
        std::string
        render_single_page(const std::filesystem::path& tmpl_path, const nlohmann::json& data);
    };

}
