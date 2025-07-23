#pragma once

#include "connection.hpp"
#include "../../core/awaitable.hpp"
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

        // Called by base class
        // ToDo: Return error code!
        awaitable<void>
        hook_connected()
        {
            // Perform the TLS handshake
            parent_t::set_stream_timeout(std::chrono::seconds(30));  // ToDo: Do not hard-code!
            co_await m_stream.async_handshake(boost::asio::ssl::stream_base::client);
        }

        /**
         * Set the hostname.
         *
         * @details This is used/needed for:
         *   - SNI (Server Name Indication) on the server side
         *   - Hostname certificate verification on the client side
         */
        malloy::error_code
        set_hostname(const std::string_view hostname)
        {
            // Note: We copy the std::string_view into an std::string as the underlying OpenSSL API expects C-strings.
            const std::string str{ hostname };

            // Specify SNI hostname (This is used by many hosts to figure out to which host we actually want to talk to)
            if (!SSL_set_tlsext_host_name(m_stream.native_handle(), str.c_str()))
                return {static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};

            // Specify the expected hostname for the peer certificate verification
            if (!SSL_set1_host(m_stream.native_handle(), str.c_str())) {
                return {static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
            }

            return { };
        }

    private:
        boost::beast::ssl_stream<malloy::tcp::stream<>> m_stream;
    };
}
