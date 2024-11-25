#pragma once

#include "../type_traits.hpp"
#include "../../core/type_traits.hpp"
#include "../../core/http/request.hpp"
#include "../../core/http/response.hpp"
#include "../../core/http/type_traits.hpp"

#include <boost/asio/strand.hpp>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <spdlog/logger.h>

#include <coroutine>
#include <future>
#include <optional>



#include <iostream>

namespace malloy::client::http
{

    /**
     * An HTTP connection.
     *
     * @tparam Derived The type inheriting from this class.
     */
    template<class Derived, malloy::http::concepts::body ReqBody, concepts::response_filter Filter, typename Callback>
    class connection
    {
    public:
        using resp_t = typename Filter::response_type;
        using callback_t = Callback;

        connection(std::shared_ptr<spdlog::logger> logger, boost::asio::io_context& io_ctx, const std::uint64_t body_limit) :
            m_logger(std::move(logger)),
            m_io_ctx(io_ctx)
        {
            // Sanity check
            if (!m_logger)
                throw std::invalid_argument("no valid logger provided.");

            // Set body limit
            m_parser.body_limit(body_limit);
        }

        // Start the asynchronous operation
        void
        run(
            char const* port,
            malloy::http::request<ReqBody> req,
            std::promise<malloy::error_code> err_channel,
            callback_t&& cb,
            Filter&& filter
        )
        {
            boost::asio::co_spawn(
                m_io_ctx,
                run_coro(std::string(port), std::move(req), std::move(err_channel), std::move(cb), std::move(filter)),
                [me = derived().shared_from_this()](std::exception_ptr e) {
                    if (e)
                        std::rethrow_exception(e);
                }
            );
        }

        boost::asio::awaitable<void>
        run_coro(
            std::string port,   // ToDo
            malloy::http::request<ReqBody> req,
            std::promise<malloy::error_code> err_channel,
            callback_t&& cb,
            Filter&& filter
        )
        {
            namespace http = boost::beast::http;

            auto executor = co_await boost::asio::this_coro::executor;
            auto resolver = boost::asio::ip::tcp::resolver{ executor };
            //auto stream   = boost::beast::tcp_stream{ executor };

            // Look up the domain name
            auto const results = co_await resolver.async_resolve(req.base()[malloy::http::field::host], port);

            // Set the timeout.
            derived().stream().expires_after(std::chrono::seconds(30));

            // Make the connection on the IP address we get from a lookup
            co_await derived().stream().async_connect(results);

            // Call "connected" hook
            co_await derived().hook_connected();

            // Set the timeout.
            derived().stream().expires_after(std::chrono::seconds(30));

            // Send the HTTP request to the remote host
            co_await http::async_write(derived().stream(), req);

            // This buffer is used for reading and must be persisted
            //boost::beast::flat_buffer buffer;

            // Declare a container to hold the response
            //http::response<http::dynamic_body> res;

            // Receive the HTTP response
            //co_await http::async_read(derived().stream(), buffer, res);

#if 0
            (void)cb;
            (void)filter;

            // Write the message to standard out
            std::cout << res << std::endl;
#else
            // ToDo
            m_req_filter = std::move(filter);
            //m_req = std::move(req);
            m_cb.emplace(std::move(cb));

            // Pick a body and parse it from the stream
            // ToDo: Have a look at using boost::beast::http::response<boost::beast::http::dynamic_body> instead!
            auto bodies = filter.body_for(m_parser.get().base());
            co_await std::visit([this](auto&& body) -> boost::asio::awaitable<void> {
                    using body_t = std::decay_t<decltype(body)>;

                    auto parser = std::make_shared<boost::beast::http::response_parser<body_t>>(std::move(m_parser));
                    m_req_filter.setup_body(parser->get().base(), parser->get().body());

                    co_await boost::beast::http::async_read(
                        derived().stream(),
                        m_buffer,
                        *parser,
                        boost::asio::use_awaitable);

                    (*m_cb)(malloy::http::response<body_t>{parser->release()});
                },
                std::move(bodies)
            );
#endif

            // Gracefully close the socket
            boost::beast::error_code ec;
            derived().stream().socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

            // not_connected happens sometimes
            // so don't bother reporting it.
            //
            if (ec && ec != boost::beast::errc::not_connected)
                m_logger->error("shutdown: {}", ec.message());

            // If we get here then the connection is closed gracefully

            // Set error channel it even if it's not an error, to signify that we are done
            err_channel.set_value(ec);
        }

    protected:
        std::shared_ptr<spdlog::logger> m_logger;

    private:
        boost::asio::io_context& m_io_ctx;
        boost::beast::http::response_parser<boost::beast::http::empty_body> m_parser;
        std::optional<callback_t> m_cb;         // ToDo: Get rid of this, no longer required
        Filter m_req_filter;                    // ToDo: Get rid of this, no longer required
        boost::beast::flat_buffer m_buffer;     // ToDo: Get rid of this, no longer required

        [[nodiscard]]
        constexpr
        Derived&
        derived() noexcept
        {
            return static_cast<Derived&>(*this);
        }
    };

}
