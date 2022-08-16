#include "connection_detector.hpp"
#include "connection_plain.hpp"
#include "../routing/router.hpp"

#if MALLOY_FEATURE_TLS
    #include "connection_tls.hpp"
#endif

#include <spdlog/logger.h>

using namespace malloy::server::http;

/**
 * Router adaptor.
 *
 * @details This is needed to break the dependency cycle between connection and router.
 *
 * @sa connection
 */
// ToDo: Consider adding public interface(s) to the connection class to facilitate logging of requests and repsonses
//       instead of passing the logger into here.
template<typename Derived>
class router_adaptor :
    public connection<Derived>::handler
{
    using router_t = std::shared_ptr<malloy::server::router>;

public:
    using http_conn_t = const connection_t&;
    using req_t = std::shared_ptr<typename connection<Derived>::request_generator>;

    /**
     * Constructor.
     *
     * @param logger The logger instance. This should be the same instance passed to the connection constructor.
     * @param router The router.
     */
    router_adaptor(std::shared_ptr<spdlog::logger> logger, router_t router) :
        m_logger{ std::move(logger) },
        m_router{ std::move(router) }
    {
    }

    void
    websocket(const std::filesystem::path& root, const req_t& req, const std::shared_ptr<malloy::server::websocket::connection>& conn) override
    {
        m_logger->info("WS request: {} {}",
           boost::beast::http::to_string(req->header().method()),
           req->header().target()
       );

        handle<true>(root, req, conn);
    }

    void
    http(const std::filesystem::path& root, const req_t& req, http_conn_t conn) override
    {
        log(
            conn,
            spdlog::level::info,
            "HTTP request: {} {}",
            boost::beast::http::to_string(req->header().method()),
            req->header().target()
        );

        handle<false>(root, req, conn);
    }

private:
    std::shared_ptr<spdlog::logger> m_logger;   // This should be the same instance as the logger used by the connection
    router_t m_router;

    template<bool isWebsocket>
    void
    handle(const std::filesystem::path& root, const req_t& req, auto conn)
    {
        m_router->handle_request<isWebsocket, Derived>(root, req, conn);
    }
};

connection_detector::connection_detector(
    std::shared_ptr<spdlog::logger> logger,
    boost::asio::ip::tcp::socket&& socket,
    std::shared_ptr<boost::asio::ssl::context> ctx,
    std::shared_ptr<const std::filesystem::path> doc_root,
    std::shared_ptr<malloy::server::router> router,
    std::string agent_string
) :
    m_logger(std::move(logger)),
    m_stream(std::move(socket)),
    m_ctx(std::move(ctx)),
    m_doc_root(std::move(doc_root)),
    m_router(std::move(router)),
    m_agent_string{std::move(agent_string)}
{
    // Sanity check logger
    if (!m_logger)
        throw std::invalid_argument("no valid logger provided.");
}

void
connection_detector::run()
{
    // Set the timeout.
    boost::beast::get_lowest_layer(m_stream).expires_after(std::chrono::seconds(30));

    // Detect a TLS handshake
    boost::beast::async_detect_ssl(
        m_stream,
        m_buffer,
        boost::beast::bind_front_handler(
            &connection_detector::on_detect,
            shared_from_this()
        )
    );
}

void
connection_detector::on_detect(boost::beast::error_code ec, [[maybe_unused]] bool result)
{
    if (ec) {
        m_logger->critical("connection type detection error: {}", ec.message());
        return;
    }

    // ToDo: Check whether it's okay to fall back to a plain session if a handshake was detected
    //       Currently we'd do this if no TLS context was provided.

    [&, this](auto&& cb) {
#if MALLOY_FEATURE_TLS
        if (result && m_ctx) {
            // Launch TLS connection
            cb(
                std::make_shared<connection_tls>(
                    m_logger,
                    m_stream.release_socket(),
                    m_ctx,
                    std::move(m_buffer),
                    m_doc_root,
                    std::make_shared<router_adaptor<connection_tls>>(m_logger, m_router)
                )
            );
        }
#endif

        cb(
            std::make_shared<connection_plain>(
                m_logger,
                m_stream.release_socket(),
                std::move(m_buffer),
                m_doc_root,
                std::make_shared<router_adaptor<connection_plain>>(m_logger, m_router)
            )
        );
    }([this](auto&& conn) {
        conn->cfg.agent_string = m_agent_string;
        conn->run();
    });
}
