#include "controller.hpp"
#include "listener.hpp"
#include "http/routing/router.hpp"
#if MALLOY_FEATURE_TLS
    #include "server_certificate.hpp"
#endif

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#if MALLOY_FEATURE_TLS
    #include <boost/asio/ssl/context.hpp>
#endif

#include <memory>

using namespace malloy::server;

controller::~controller()
{
    stop().wait();
}

bool controller::init(config cfg)
{
    // Don't re-initialize
    if (m_init_done)
        return false;
    m_init_done = true;

    // Create a logger if none was provided
    if (not cfg.logger)
    {
        auto log_level = spdlog::level::trace;

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

    // Create the top-level router
    m_router = std::make_shared<malloy::http::server::router>(m_cfg.logger->clone("router"));

    return true;
}

#if MALLOY_FEATURE_TLS
    bool controller::init_tls()
    {
        m_tls_ctx = std::make_shared<boost::asio::ssl::context>( boost::asio::ssl::context::tlsv12 );

        // This holds the self-signed certificate used by the server
        load_server_certificate(*m_tls_ctx);

        return true;
    }
#endif

bool controller::start()
{
    // Must be initialized
    if (not m_init_done)
        return false;

    // Create the listener
    m_listener = std::make_shared<malloy::server::listener>(
        m_cfg.logger->clone("listener"),
        m_io_ctx,
        m_tls_ctx,
        boost::asio::ip::tcp::endpoint{ boost::asio::ip::make_address(m_cfg.interface), m_cfg.port },
        m_router,
        std::make_shared<std::filesystem::path>(m_cfg.doc_root)
    );

    // Run the listener
    m_listener->run();

    // Create the I/O context threads
    m_threads.reserve(m_cfg.num_threads - 1);
    for (std::size_t i = 0; i < m_cfg.num_threads; i++) {
        m_threads.emplace_back(
            [this]
            {
                m_io_ctx.run();
            }
        );
    }

    // Log
    m_cfg.logger->info("starting server.");

    // Start the I/O context
    m_io_ctx.run();

    return true;
}

std::future<void> controller::stop()
{
    // Stop the `io_context`. This will cause `run()`
    // to return immediately, eventually destroying the
    // `io_context` and all of the sockets in it.
    m_io_ctx.stop();

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
    if (m_listener)
        m_listener->set_websocket_handler(std::move(handler));
}
