#pragma once

#include "connection.hpp"

namespace malloy::client::http
{

    /**
     * A plain HTTP connection.
     */

    template<typename... ConnArgs>
    class connection_plain :
        public connection<connection_plain<ConnArgs...>, ConnArgs...>,
        public std::enable_shared_from_this<connection_plain<ConnArgs...>>
    {
        using parent_t = connection<connection_plain<ConnArgs...>, ConnArgs...>;
    public:
        connection_plain(std::shared_ptr<spdlog::logger> logger, boost::asio::io_context& io_ctx) :
            parent_t(std::move(logger), io_ctx),
            m_stream(boost::asio::make_strand(io_ctx))
        {
        }

        // Called by base class
        [[nodiscard]]
        boost::beast::tcp_stream&
        stream()
        {
            return m_stream;
        }

        void
        hook_connected()
        {
            // Send the HTTP request to the remote host
            parent_t::send_request();
        }

    private:
        boost::beast::tcp_stream m_stream;
    };
}
