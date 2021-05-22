#pragma once

#include "connection.hpp"

#include <boost/beast/ssl.hpp>

namespace malloy::server::websocket
{

    /**
     * Handles a TLS encrypted websocket connection/session.
     */
    class connection_tls :
        public connection<connection_tls>,
        public std::enable_shared_from_this<connection_tls>
    {
    public:
        connection_tls(
            std::shared_ptr<spdlog::logger> logger,
            boost::beast::ssl_stream<boost::beast::tcp_stream>&& stream
        ) :
            connection(std::move(logger)),
            m_stream(std::move(stream))
        {
            // We operate in binary mode
            m_stream.binary(true);
        }

        // Called by the base class
        [[nodiscard]]
        boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>>&
        stream()
        {
            return m_stream;
        }

    private:
        boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>> m_stream;
    };

    template<class Body, class Allocator>
    std::shared_ptr<connection_tls>
    make_websocket_connection(
        std::shared_ptr<spdlog::logger> logger,
        boost::beast::ssl_stream<boost::beast::tcp_stream> stream,
        boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req
    )
    {
        return std::make_shared<connection_tls>(
            std::move(logger),
            std::move(stream)
        );
    }

}
