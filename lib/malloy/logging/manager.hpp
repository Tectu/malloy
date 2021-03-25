#pragma once

#include <memory>

namespace spdlog
{
    class logger;
}

namespace logging
{

    class manager
    {
    public:
        // Construction
        manager(const manager& other) = delete;
        manager(manager&& other) = delete;

        // Operators
        manager& operator=(const manager& rhs) = delete;
        manager& operator=(manager&& rhs) = delete;

        // Singleton
        [[nodiscard]]
        static manager& instance() noexcept;

        // General
        [[nodiscard]]
        std::shared_ptr<spdlog::logger> make_logger(const std::string& name) const;

    private:
        std::shared_ptr<spdlog::logger> m_logger;

        manager();
        virtual ~manager() = default;
    };

}
