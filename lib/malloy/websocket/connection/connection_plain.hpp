#pragma once

#include "connection.hpp"

#include <boost/beast/websocket/stream.hpp>

namespace malloy::websocket::server
{

    /**
     * Handles a plain websocket connection/session.
     */
    class connection_plain :
        public connection<connection_plain>,
        public std::enable_shared_from_this<connection_plain>
    {

    public:
        connection_plain(
            std::shared_ptr<spdlog::logger> logger,
            handler_type handler,
            boost::beast::tcp_stream&& stream
        ) :
            connection(
                std::move(logger),
                std::move(handler)
            ),
            m_stream(std::move(stream))
        {
            // We operate in binary mode
            m_stream.binary(true);
        }

        // Called by the base class
        [[nodiscard]]
        boost::beast::websocket::stream<boost::beast::tcp_stream>&
        stream()
        {
            return m_stream;
        }

    private:
        boost::beast::websocket::stream<boost::beast::tcp_stream> m_stream;
    };

    template<class Body, class Allocator>
    void
    make_websocket_connection(
        std::shared_ptr<spdlog::logger> logger,
        handler_type handler,
        boost::beast::tcp_stream stream,
        boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req
    )
    {
        std::make_shared<connection_plain>(
            std::move(logger),
            std::move(handler),
            std::move(stream))->run(std::move(req)
        );
    }

}
