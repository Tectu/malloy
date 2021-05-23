#pragma once

#include "connection.hpp"

#include <boost/beast/websocket/stream.hpp>

namespace malloy::server::websocket
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
            boost::beast::tcp_stream&& stream
        ) :
            connection(std::move(logger)),
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

    std::shared_ptr<connection_plain>
    make_websocket_connection(
        std::shared_ptr<spdlog::logger> logger,
        boost::beast::tcp_stream stream
    )
    {
        return std::make_shared<connection_plain>(
            std::move(logger),
            std::move(stream)
        );
    }

}
