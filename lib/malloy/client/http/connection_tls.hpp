#pragma once

#include "connection.hpp"
#include "../../core/tcp/stream.hpp"

#include <boost/beast/ssl/ssl_stream.hpp>

namespace malloy::client::http
{

    /**
     * A TLS (SSL) HTTPS connection.
     */
    template<typename... ConnArgs>
    class connection_tls :
        public connection<connection_tls<ConnArgs...>, ConnArgs...>,
        public std::enable_shared_from_this<connection_tls<ConnArgs...>>
    {
        using parent_t = connection<connection_tls<ConnArgs...>, ConnArgs...>;

    public:
        connection_tls(
            std::shared_ptr<spdlog::logger> logger,
            boost::asio::io_context& io_ctx,
            boost::asio::ssl::context& tls_ctx,
            const std::uint64_t body_limit
        ) :
            parent_t(std::move(logger), io_ctx, body_limit),
            m_stream(boost::asio::make_strand(io_ctx), tls_ctx)
        {
        }

        // Called by base class
        [[nodiscard]]
        boost::beast::ssl_stream<malloy::tcp::stream<>>&
        stream()
        {
            return m_stream;
        }

        void
        hook_connected()
        {
            // Perform the TLS handshake
            m_stream.async_handshake(
                boost::asio::ssl::stream_base::client,
                boost::beast::bind_front_handler(
                    &connection_tls::on_handshake,
                    this->shared_from_this()
                )
            );
        }

    private:
        boost::beast::ssl_stream<malloy::tcp::stream<>> m_stream;

        void
        on_handshake(const boost::beast::error_code ec)
        {
            if (ec)
                return parent_t::m_logger->error("on_handshake(): {}", ec.message());

            // Set a timeout on the operation
            boost::beast::get_lowest_layer(m_stream).expires_after(std::chrono::seconds(30));

            // Send the HTTP request to the remote host
            parent_t::send_request();
        }
    };
}
