#pragma once

#include "connection.hpp"
#include "../../core/tcp/stream.hpp"

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
        connection_plain(std::shared_ptr<spdlog::logger> logger, boost::asio::io_context& io_ctx, const std::uint64_t body_limit) :
            parent_t(std::move(logger), io_ctx, body_limit),
            m_stream(boost::asio::make_strand(io_ctx))
        {
        }

        // Called by base class
        [[nodiscard]]
        malloy::tcp::stream&
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
        malloy::tcp::stream m_stream;
    };
}
