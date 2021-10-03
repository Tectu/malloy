#pragma once

#include "../routing/router.hpp"
#include "environment.hpp"

#include <spdlog/logger.h>

#include <concepts>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace malloy::server::app_fw
{
    class page;

    /**
     * A base class for an application.
     */
    class app
    {
    public:
        app(
            std::shared_ptr<spdlog::logger> logger,
            std::string name,
            environment env
        );

        app() = default;
        app(const app&) = delete;
        app(app&&) noexcept = delete;
        virtual ~app() noexcept = default;

        app& operator=(const app&) = delete;
        app& operator=(app&&) noexcept = delete;

        [[nodiscard]]
        virtual
        bool
        init() = 0;

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
        environment m_env;
        std::shared_ptr<spdlog::logger> m_logger;
        std::shared_ptr<malloy::server::router> m_router;

        /**
         * Adds an endpoint for an HTML page.
         *
         * @param target The target path.
         * @param page The HTML page.
         * @return Whether adding the endpoint was successful.
         */
        bool
        add_page(std::string&& resource, std::shared_ptr<page>);

        /**
         * Makes a new sub-application of type @ref App.
         *
         * @tparam App The app type.
         * @tparam Args `App` constructor type list.
         * @param name The name of the new sub-app.
         * @param args The arguments passed to the contructor of the new sub-app.
         * @return Whether making the sub-app was successful.
         */
        template<class App, typename... Args>
            requires std::derived_from<App, app>
        bool
        make_subapp(const std::string& name, Args&&... args)
        {
            // Sanity check name
            if (name.empty()) {
                m_logger->error("could not make sub-app. name must not be empty.");
                return false;
            }

            // Create & setup app
            auto app = std::make_shared<App>(std::forward<Args>(args)...);
            app->m_name = name;
            app->m_env = m_env.make_sub_environment(name);
            app->m_logger = m_logger->clone(m_logger->name() + " | " + name);
            app->m_router = std::make_shared<malloy::server::router>();

            // Initialize app
            if (!app->init()) {
                m_logger->error("initialization of sub-app \"{}\" failed.", name);
                return false;
            }

            // Add the sub-router
            const std::string target_base = "/" + name;
            if (!m_router->add_subrouter(target_base, app->router())) {
                m_logger->error("could not add router of sub-app");
                return false;
            }

            // House keeping
            m_subapps.emplace_back(std::move(app));

            return true;
        }

    private:
        std::string m_name;
        std::vector<std::shared_ptr<app>> m_subapps;
    };

}
