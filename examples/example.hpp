#pragma once

#include <malloy/server/routing_context.hpp>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <filesystem>

/**
 * HTTP document root path
 */
const std::filesystem::path examples_doc_root = "../../examples/server/static_content";

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

/**
 * Setup the malloy config with values suitable for examples.
 *
 * @param cfg The config.
 */
static
void
setup_example_config(malloy::server::routing_context::config& cfg)
{
    cfg.doc_root          = examples_doc_root;
    cfg.num_threads       = 5;
    cfg.logger            = create_example_logger();
    cfg.connection_logger = cfg.logger;
}
