#include "controller.hpp"
#include "websocket/connection_plain.hpp"

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#if MALLOY_FEATURE_TLS
    #include <boost/beast/ssl.hpp>
    #include <boost/certify/extensions.hpp>
    #include <boost/certify/https_verification.hpp>
#endif
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <algorithm>

using namespace malloy::client;

controller::~controller()
{
    stop().wait();
}

bool controller::init(config cfg)
{
    // Don't re-initialize
    static bool m_init_done = false;
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

    // Create the I/O context
    m_io_ctx = std::make_shared<boost::asio::io_context>();

    // Create a worker thread to run the boost::asio::io_context.
    // The work guard is used to prevent the io_context::run() from returning if there's no work scheduled.
    //boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_work_guard = boost::asio::make_work_guard(*m_io_ctx);

    //m_workguard = std::make_unique<workguard_t>(m_io_ctx->get_executor());
    m_workguard = std::make_unique<workguard_t>(boost::asio::make_work_guard(*m_io_ctx));

    return true;
}

bool controller::start()
{
    // Sanity check
    if (!m_io_ctx) {
        m_cfg.logger->critical("no I/O context present. Make sure that init() was called and succeeded.");
        return false;
    }

    // Create the I/O context threads
    m_io_threads.reserve(m_cfg.num_threads - 1);
    for (std::size_t i = 0; i < m_cfg.num_threads; i++) {
        m_io_threads.emplace_back(
            [this]
            {
                m_io_ctx->run();
            }
        );
    }

    // Log
    m_cfg.logger->info("starting i/o context.");

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

            for (auto& thread : m_io_threads)
                thread.join();

            m_cfg.logger->info("all I/O threads stopped.");
        }
    );
}

std::shared_ptr<websocket::connection_plain>
controller::add_connection(std::string id, const std::string& host, std::uint16_t port, const std::string& endpoint, malloy::websocket::handler_t&& handler)
{
    // Sanity check
    if (!handler)
        return { };

    // Create connection
    auto conn = std::make_shared<websocket::connection_plain>(m_cfg.logger->clone("connection"), *m_io_ctx, std::move(handler));

    // Launch the connection
    conn->connect(host, std::to_string(port), endpoint);

    // Store
    m_connections.try_emplace(std::move(id), conn);

    return conn;
}

std::vector<std::string> controller::connections() const
{
    std::vector<std::string> ret;
    ret.reserve(m_connections.size());
    std::transform(
        std::cbegin(m_connections),
        std::cend(m_connections),
        std::back_inserter(ret),
        [](const auto& pair) {
            return pair.first;
        }
    );

    return ret;
}

std::shared_ptr<websocket::connection_plain>
controller::connection(const std::string& id)
{
    if (!m_connections.contains(id))
        return { };

    return m_connections.at(id);
}

void controller::test_tls()
{
    /*
    std::string host = "127.0.0.1";
    std::string port = "8080";
    std::string endpoint = "/echo";
    std::string text = "Hello malloy client [tls]";

    // The io_context is required for all I/O
    boost::asio::io_context ioc;

    // The SSL context is required, and holds certificates
    boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};
    ctx.set_verify_mode(boost::asio::ssl::verify_peer | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
    ctx.set_default_verify_paths();
    boost::certify::enable_native_https_server_verification(ctx);

    // Launch the asynchronous operation
    std::make_shared<connection_tls>(ioc, ctx)->run(host, port, endpoint, text);

    // Run the I/O service. The call will return when
    // the socket is closed.
    ioc.run();
     */
}
