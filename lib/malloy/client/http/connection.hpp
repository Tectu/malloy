#pragma once

#include "request_result.hpp"
#include "../type_traits.hpp"
#include "../../core/mp.hpp"
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

namespace malloy::client::http
{

    /**
     * An HTTP connection.
     *
     * @tparam Derived The type inheriting from this class.
     */
    template<
        class Derived,
        malloy::http::concepts::body ReqBody,
        concepts::response_filter Filter
    >
    class connection
    {
    public:
        connection(std::shared_ptr<spdlog::logger> logger, const std::uint64_t body_limit) :
            m_logger(std::move(logger))
        {
            // Sanity check
            if (!m_logger)
                throw std::invalid_argument("no valid logger provided.");

            // Set body limit
            m_parser.body_limit(body_limit);
        }

        // Note: We pass by copy as we need to make sure that these members stay alive until the coroutine co_returns.
        //       The consuming application passes in the request and filter via malloy::client::controller::http_request(). If those callables
        //       are for example a lambda, they are being destroyed once the http_request() function returns. http_request() will actually return
        //       before the execution of this run() function completed.
        // ToDo: Don't deal with ec1, ec2, ec3 and so on. Reuse the same ec instance each time.
        boost::asio::awaitable< request_result<Filter> >
        run(
            malloy::http::request<ReqBody> req,
            Filter filter
        )
        {
            // Get the executor
            auto executor = co_await boost::asio::this_coro::executor;

            // Look up the domain name
            auto resolver = boost::asio::ip::tcp::resolver{ executor };
            const auto [ec1, results] = co_await resolver.async_resolve(
                req.base()[malloy::http::field::host],
                std::to_string(req.port()),
                boost::asio::as_tuple(boost::asio::use_awaitable)
            );
            if (ec1)
                co_return std::unexpected(ec1);

            // Make the connection on the IP address we get from a lookup
            malloy::error_code ec2;
            set_stream_timeout(std::chrono::seconds(30));   // ToDo: Don't hard-code
            co_await boost::beast::get_lowest_layer(derived().stream()).async_connect(results, boost::asio::redirect_error(boost::asio::use_awaitable, ec2));
            if (ec2)
                co_return std::unexpected(ec2);

            // Call "connected" hook
            co_await derived().hook_connected();

            // Send the HTTP request to the remote host
            set_stream_timeout(std::chrono::seconds(30));   // ToDo: Don't hard-code
            co_await boost::beast::http::async_write(derived().stream(), req);

            // Pick a body and parse it from the stream
            // ToDo: Have a look at using boost::beast::http::response<boost::beast::http::dynamic_body> instead!
            //       This would probably involve something like:
            //
            //          boost::beast::flat_buffer buffer;
            //          http::response<http::dynamic_body> res;
            //          co_await http::async_read(derived().stream(), buffer, res);
            //
            auto bodies = filter.body_for(m_parser.get().base());
            auto resp = co_await std::visit(
                [&filter, this](auto&& body) -> boost::asio::awaitable< malloy::mp::filter_resp_t<Filter> > {
                    using body_t = std::decay_t<decltype(body)>;

                    auto parser = std::make_shared<boost::beast::http::response_parser<body_t>>(std::move(m_parser));
                    filter.setup_body(parser->get().base(), parser->get().body());

                    boost::beast::flat_buffer buffer;

                    co_await boost::beast::http::async_read(
                        derived().stream(),
                        buffer,
                        *parser,
                        boost::asio::use_awaitable
                    );

                    co_return malloy::http::response<body_t>{ parser->release() };
                },
                std::move(bodies)
            );

            // Gracefully close the socket
            malloy::error_code ec;
            set_stream_timeout(std::chrono::seconds(30));   // ToDo: Don't hard-code
            // ToDo: This should be co_await too!
            boost::beast::get_lowest_layer(derived().stream()).socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

            // not_connected happens sometimes
            // so don't bother reporting it.
            //
            if (ec && ec != boost::beast::errc::not_connected)
                m_logger->error("shutdown: {}", ec.message());

            // If we get here then the connection is closed gracefully

            co_return std::move(resp);  // ToDo: Move correct?
        }

    protected:
        std::shared_ptr<spdlog::logger> m_logger;

        template<typename Rep, typename Period>
        void
        set_stream_timeout(const std::chrono::duration<Rep, Period> duration)
        {
            // ToDo: Which one is correct?!?!

            //derived().stream().expires_after(duration);
            boost::beast::get_lowest_layer(derived().stream()).expires_after(duration);
        }

    private:
        boost::beast::http::response_parser<boost::beast::http::empty_body> m_parser;

        [[nodiscard]]
        constexpr
        Derived&
        derived() noexcept
        {
            return static_cast<Derived&>(*this);
        }
    };

}
