#pragma once

#include "page.hpp"

#include <nlohmann/json.hpp>

#include <filesystem>

namespace malloy::server::app_fw
{
    class page_master;

    /**
     * This type represents the content portion of a page.
     */
    class page_content :
        public page
    {
    public:
        page_content(
            std::filesystem::path template_path,
            std::shared_ptr<page_master> master
        );

        page_content() = delete;
        ~page_content() noexcept override = default;

        [[nodiscard]]
        std::filesystem::path
        template_path() const noexcept
        {
            return m_template_path;
        }

        [[nodiscard]]
        virtual
        nlohmann::json
        data() const = 0;

        [[nodiscard]]
        malloy::http::response<>
        render() const override;

    private:
        std::filesystem::path m_template_path;
        std::shared_ptr<page_master> m_master;
    };

}
