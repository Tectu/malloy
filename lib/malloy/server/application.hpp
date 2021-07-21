#pragma once

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

    /**
     * A base class for an application.
     */
    class application
    {
    public:
        application(
            std::shared_ptr<spdlog::logger> logger,
            std::string name
        );

        application(const application&) = delete;
        application(application&&) noexcept = delete;
        virtual ~application() noexcept = default;

        application& operator=(const application&) = delete;
        application& operator=(application&&) noexcept = delete;

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

        [[nodiscard]]
        std::shared_ptr<application>
        make_subapp(const std::string& name, std::string target_base);

    private:
        std::string m_name;
        std::vector<std::shared_ptr<application>> m_subapps;
    };

}
