#pragma once

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>

namespace spdlog
{
    class logger;
}

namespace logging
{

    class logger
    {
    public:
        // Construction
        logger(const logger& other) = delete;
        logger(logger&& other) = delete;

        // Operators
        logger& operator=(const logger& rhs) = delete;
        logger& operator=(logger&& rhs) = delete;

        // Singleton
        [[nodiscard]]
        static logger& instance() noexcept
        {
            static logger inst;

            return inst;
        }

        // General
        [[nodiscard]]
        std::shared_ptr<spdlog::logger> make_logger(const std::string& name) const
        {
            if (not m_logger)
                return nullptr;

            return m_logger->clone(name);
        }

    private:
        std::shared_ptr<spdlog::logger> m_logger;

        logger()
        {
            auto log_level = spdlog::level::trace;

            // Sink
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(log_level);

            // Create logger
            m_logger = std::make_shared<spdlog::logger>("", spdlog::sinks_init_list{ console_sink });
            m_logger->set_level(log_level);
        }

        virtual ~logger() = default;
    };

}
