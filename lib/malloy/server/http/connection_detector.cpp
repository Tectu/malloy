#include "connection_detector.hpp"
#include "connection_plain.hpp"
#include "../routing/router.hpp"

#if MALLOY_FEATURE_TLS
    #include "connection_tls.hpp"
#endif

using namespace malloy::server::http;

connection_detector::connection_detector(
    std::shared_ptr<spdlog::logger> logger,
    boost::asio::ip::tcp::socket&& socket,
    std::shared_ptr<boost::asio::ssl::context> ctx,
    std::shared_ptr<const std::filesystem::path> doc_root,
    std::shared_ptr<malloy::server::router> router
) :
    m_logger(std::move(logger)),
    m_stream(std::move(socket)),
    m_ctx(std::move(ctx)),
    m_doc_root(std::move(doc_root)),
    m_router(std::move(router))
{
    // Sanity check logger
    if (!m_logger)
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

/// This is needed to break the dependency cycle between connection and router
template<typename Derived>
class router_adaptor: public connection<Derived>::handler {
    using router_t = std::shared_ptr<malloy::server::router>;
public:
    using conn_t = const connection_t&;
    using req_t = std::shared_ptr<typename connection<Derived>::request_generator>;

    router_adaptor(router_t router) : router_{std::move(router)} {}

    void websocket(const std::filesystem::path& root, const req_t& req, const std::shared_ptr<malloy::server::websocket::connection>& conn) override { 
        send_msg<true>(root, req, conn); 
    }
    void http(const std::filesystem::path& root, const req_t& req, conn_t conn) override { 
        send_msg<false>(root, req, conn); 
    }
private:
    template<bool isWs>
    void send_msg(const std::filesystem::path& root, const req_t& req, auto conn) {
        router_->handle_request<isWs, Derived>(root, req, conn); 
    }

    router_t router_;
};

void connection_detector::on_detect(boost::beast::error_code ec, bool result)
{
    if (ec) {
        m_logger->critical("connection type detection error: {}", ec.message());
        return;
    }

    // ToDo: Check whether it's okay to fall back to a plain session if a handshake was detected
    //       Currently we'd do this if no TLS context was provided.

    #if MALLOY_FEATURE_TLS
        if (result && m_ctx) {
            // Launch TLS connection
            std::make_shared<connection_tls>(
                m_logger,
                m_stream.release_socket(),
                m_ctx,
                std::move(m_buffer),
                m_doc_root,
                std::make_shared<router_adaptor<connection_tls>>(m_router)
            )->run();

            return;
        }
    #endif

    // Launch plain connection
    std::make_shared<connection_plain>(
        m_logger,
        m_stream.release_socket(),
        std::move(m_buffer),
        m_doc_root,
        std::make_shared<router_adaptor<connection_plain>>(m_router)
    )->run();
}
