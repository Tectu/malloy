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
            parent_t(std::move(logger), body_limit),
            m_stream(boost::asio::make_strand(io_ctx))      // ToDo: make_strand() necessary since we're using coroutines?
        {
        }

        // Called by base class
        [[nodiscard]]
        constexpr
        malloy::tcp::stream<>&
        stream() noexcept
        {
            return m_stream;
        }

        // Called by base class
        boost::asio::awaitable<void>
        hook_connected()
        {
            co_return;
        }

    private:
        malloy::tcp::stream<> m_stream;
    };
}
