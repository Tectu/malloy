#pragma once

#include "connection.hpp"

#include <boost/beast/ssl/ssl_stream.hpp>

namespace malloy::client::http
{

    /**
     * A TLS (SSL) HTTPS connection.
     */
    class connection_tls :
        public connection<connection_tls>,
        public std::enable_shared_from_this<connection_tls>
    {
    public:
        connection_tls(
            std::shared_ptr<spdlog::logger> logger,
            boost::asio::io_context& io_ctx,
            boost::asio::ssl::context& tls_ctx
        ) :
            connection(std::move(logger), io_ctx),
            m_stream(boost::asio::make_strand(io_ctx), tls_ctx)
        {
        }

        // Called by base class
        [[nodiscard]]
        boost::beast::ssl_stream<boost::beast::tcp_stream>&
        stream()
        {
            return m_stream;
        }

    private:
        boost::beast::ssl_stream<boost::beast::tcp_stream> m_stream;
    };
}
