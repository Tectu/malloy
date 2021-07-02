#pragma once

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

/**
 * Creates a logger used by the various examples.
 *
 * @return The corresponding logger.
 */
[[nodiscard]]
static
std::shared_ptr<spdlog::logger>
create_example_logger()
{
    auto log_level = spdlog::level::debug;

    // Sink
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(log_level);

    // Create logger
    auto logger = std::make_shared<spdlog::logger>("", spdlog::sinks_init_list{ console_sink });
    logger->set_level(log_level);

    return logger;
}
