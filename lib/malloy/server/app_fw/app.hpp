#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace spdlog
{
    class logger;
}

namespace malloy::server
{
    class router;
}

namespace malloy::server::app_fw
{
    class page;

    /**
     * A base class for an application.
     */
    class app
    {
    public:
        struct environment
        {
            struct {
                std::string base_url;
            } site;

            struct {
                std::string base_url;
            } app;

            [[nodiscard]]
            environment
            make_sub_environment(const std::string& name)
            {
                environment env{ *this };
                env.app.base_url += "/" + name;

                return env;
            }
        };

        app(
            std::shared_ptr<spdlog::logger> logger,
            std::string name,
            environment env
        );

        app(const app&) = delete;
        app(app&&) noexcept = delete;
        virtual ~app() noexcept = default;

        app& operator=(const app&) = delete;
        app& operator=(app&&) noexcept = delete;

        [[nodiscard]]
        std::string_view
        name() const noexcept
        {
            return m_name;
        }

        [[nodiscard]]
        std::shared_ptr<malloy::server::router>
        router() const noexcept
        {
            return m_router;
        }

    protected:
        std::shared_ptr<spdlog::logger> m_logger;
        std::shared_ptr<malloy::server::router> m_router;

        /**
         * Adds an endpoint for an HTML page.
         *
         * @param target The target path.
         * @param page The HTML page.
         * @return Whether adding the endpoint was successful.
         */
        [[nodiscard]]
        bool
        add_page(std::string&& resource, std::shared_ptr<page>);

        /**
         * Adds a sub-application
         *
         * @param app The sub-application.
         * @return Whether adding the sub-application was successful.
         */
        [[nodiscard]]
        bool
        add_subapp(std::shared_ptr<app> app);

    private:
        std::string m_name;
        environment m_env;
        std::vector<std::shared_ptr<app>> m_subapps;
    };

}
