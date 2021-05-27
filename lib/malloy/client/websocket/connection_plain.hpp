#pragma once

#include "connection.hpp"

namespace malloy::client::websocket
{

    /**
     * A plain websocket connection.
     */
    class connection_plain :
        public connection<connection_plain>,
        public std::enable_shared_from_this<connection_plain>
    {

    public:
        connection_plain(
            std::shared_ptr<spdlog::logger> logger,
            boost::asio::io_context& io_ctx,
            malloy::websocket::handler_t&& handler
        ) :
            connection(std::move(logger), io_ctx, std::move(handler)),
            m_stream(boost::asio::make_strand(io_ctx))
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

}
