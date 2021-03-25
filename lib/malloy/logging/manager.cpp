#include "manager.hpp"

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

using namespace logging;

manager::manager()
{
    auto log_level = spdlog::level::trace;

    // Sink
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(log_level);

    // Create logger
    m_logger = std::make_shared<spdlog::logger>("", spdlog::sinks_init_list{ console_sink });
    m_logger->set_level(log_level);
}

manager& manager::instance() noexcept
{
    static manager inst;

    return inst;
}

std::shared_ptr<spdlog::logger> manager::make_logger(const std::string& name) const
{
    if (not m_logger)
        return nullptr;

    return m_logger->clone(name);
}
