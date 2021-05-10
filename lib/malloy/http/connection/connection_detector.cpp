#include "connection_detector.hpp"
#include "connection_plain.hpp"

#if MALLOY_FEATURE_TLS
    #include "connection_tls.hpp"
#endif

using namespace malloy::http::server;

connection_detector::connection_detector(
    std::shared_ptr<spdlog::logger> logger,
    boost::asio::ip::tcp::socket&& socket,
    std::shared_ptr<boost::asio::ssl::context> ctx,
    std::shared_ptr<const std::filesystem::path> doc_root,
    std::shared_ptr<http::server::router> router,
    malloy::websocket::handler_type websocket_handler
) :
    m_logger(std::move(logger)),
    m_stream(std::move(socket)),
    m_ctx(std::move(ctx)),
    m_doc_root(std::move(doc_root)),
    m_router(std::move(router)),
    m_websocket_handler(std::move(websocket_handler))
{
    // Sanity check logger
    if (not m_logger)
        throw std::invalid_argument("no valid logger provided.");
}

void connection_detector::run()
{
    // Set the timeout.
    boost::beast::get_lowest_layer(m_stream).expires_after(std::chrono::seconds(30));

    // Detect a TLS handshake
    async_detect_ssl(
        m_stream,
        m_buffer,
        boost::beast::bind_front_handler(
            &connection_detector::on_detect,
            shared_from_this()
        )
    );
}

void connection_detector::on_detect(boost::beast::error_code ec, bool result)
{
    if (ec) {
        m_logger->critical("connection type detection error: {}", ec.message());
        return;
    }

    // ToDo: Check whether it's okay to fall back to a plain session if a handshake was detected
    //       Currently we'd do this if no TLS context was provided.

    #if MALLOY_FEATURE_TLS
        if (result and m_ctx) {
            // Log
            m_logger->debug("launching TLS connection.");

            // Launch TLS connection
            std::make_shared<connection_tls>(
                m_logger,
                m_stream.release_socket(),
                m_ctx,
                std::move(m_buffer),
                m_doc_root,
                m_router,
                m_websocket_handler
            )->run();

            return;
        }
    #endif

    // Launch plain connection
    m_logger->debug("launching plain connection.");
    std::make_shared<connection_plain>(
        m_logger,
        m_stream.release_socket(),
        std::move(m_buffer),
        m_doc_root,
        m_router,
        m_websocket_handler
    )->run();
}
