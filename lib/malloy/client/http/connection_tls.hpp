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
            parent_t(std::move(logger), body_limit),
            m_stream(boost::asio::make_strand(io_ctx), tls_ctx)     // ToDo: make_strand() necessary since we're using coroutines?
        {
        }

        // Called by base class
        [[nodiscard]]
        boost::beast::ssl_stream<malloy::tcp::stream<>>&
        stream()
        {
            return m_stream;
        }

        // ToDo: Return error code!
        boost::asio::awaitable<void>
        hook_connected()
        {
            // Perform the TLS handshake
            parent_t::set_stream_timeout(std::chrono::seconds(30));  // ToDo: Do not hard-code!
            co_await m_stream.async_handshake(boost::asio::ssl::stream_base::client);
        }

    private:
        boost::beast::ssl_stream<malloy::tcp::stream<>> m_stream;
    };
}
