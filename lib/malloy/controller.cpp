#include "controller.hpp"
#include "listener.hpp"
#include "http/routing/router.hpp"
#if MALLOY_FEATURE_TLS
    #include "tls/manager.hpp"
#endif

#include <boost/asio/io_context.hpp>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>

using namespace malloy::server;

controller::~controller()
{
    stop().wait();
}

bool controller::init(config cfg, std::shared_ptr<boost::asio::io_context> io_ctx)
{
    // Don't re-initialize
    if (m_init_done)
        return false;
    m_init_done = true;

    // Create a logger if none was provided
    if (not cfg.logger)
    {
        auto log_level = spdlog::level::debug;

        // Sink
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(log_level);

        // Create logger
        cfg.logger = std::make_shared<spdlog::logger>("", spdlog::sinks_init_list{ console_sink });
        cfg.logger->set_level(log_level);
    }

    // Sanity check number of threads
    if (cfg.num_threads < 1) {
        cfg.logger->error("num_threads must be >= 1");
        return false;
    }

    // Grab the config
    m_cfg = std::move(cfg);

    // Create the I/O context (if necessary)
    m_io_ctx = std::move(io_ctx);
    if (not m_io_ctx)
        m_io_ctx = std::make_shared<boost::asio::io_context>();

    // Create the top-level router
    m_router = std::make_shared<malloy::http::server::router>(m_cfg.logger->clone("router"));

    return true;
}

#if MALLOY_FEATURE_TLS
    bool controller::init_tls(
        const std::filesystem::path& cert_path,
        const std::filesystem::path& key_path
    )
    {
        // Sanity check cert
        if (not std::filesystem::is_regular_file(cert_path)) {
            m_cfg.logger->critical("could not create TLS context: invalid certificate file path: {}", cert_path.string());
            return false;
        }

        // Sanity check key_path
        if (not std::filesystem::is_regular_file(key_path)) {
            m_cfg.logger->critical("could not create TLS context: invalid key file path: {}", key_path.string());
        }

        // Create the context
        m_tls_ctx = tls::manager::make_context(cert_path, key_path);

        return true;
    }
#endif

bool controller::start()
{
    // Must be initialized
    if (not m_init_done)
        return false;

    // Sanity check
    assert(m_io_ctx);

    // Create the listener
    m_listener = std::make_shared<malloy::server::listener>(
        m_cfg.logger->clone("listener"),
        *m_io_ctx,
        m_tls_ctx,
        boost::asio::ip::tcp::endpoint{ boost::asio::ip::make_address(m_cfg.interface), m_cfg.port },
        m_router,
        std::make_shared<std::filesystem::path>(m_cfg.doc_root),
        m_websocket_handler
    );

    // Run the listener
    m_listener->run();

    // Create the I/O context threads
    m_threads.reserve(m_cfg.num_threads - 1);
    for (std::size_t i = 0; i < m_cfg.num_threads; i++) {
        m_threads.emplace_back(
            [this]
            {
                m_io_ctx->run();
            }
        );
    }

    // Log
    m_cfg.logger->info("starting server.");

    // Start the I/O context
    m_io_ctx->run();

    return true;
}

std::future<void> controller::stop()
{
    // Stop the `io_context`. This will cause `run()`
    // to return immediately, eventually destroying the
    // `io_context` and all of the sockets in it.
    m_io_ctx->stop();

    // Wait for all I/O context threads to finish...
    return std::async(
        [this]
        {
            m_cfg.logger->info("waiting for I/O threads to stop...");

            for (auto& thread : m_threads)
                thread.join();

            m_cfg.logger->info("all I/O threads stopped.");
        }
    );
}

void controller::set_websocket_handler(malloy::websocket::handler_type handler)
{
    m_websocket_handler = std::move(handler);
}
