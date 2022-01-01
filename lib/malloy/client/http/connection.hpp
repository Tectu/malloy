#pragma once

#include "../type_traits.hpp"
#include "../../core/type_traits.hpp"
#include "../../core/http/request.hpp"
#include "../../core/http/response.hpp"
#include "../../core/http/type_traits.hpp"

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <spdlog/logger.h>

#include <future>
#include <optional>

namespace malloy::client::http
{

    /**
     * An HTTP connection.
     *
     * @tparam Derived The type inheriting from this class.
     */
    template<class Derived, malloy::http::concepts::body ReqBody, concepts::response_filter Filter, std::invocable<error_code, tmp::filter_resp_t<Filter>> Callback>
    class connection
    {
    public:
        using resp_t = typename Filter::response_type;
        using callback_t = Callback;

        connection(std::shared_ptr<spdlog::logger> logger, boost::asio::io_context& io_ctx, callback_t cb, std::string port, malloy::http::request<ReqBody> req, Filter&& filter) :
            m_logger(std::move(logger)),
            m_resolver(boost::asio::make_strand(io_ctx)),
            m_port{std::move(port)},
            m_req_filter{std::move(filter)},
            m_req{std::move(req)},
            m_cb{std::move(cb)}
        {
            // Sanity check
            if (!m_logger)
                throw std::invalid_argument("no valid logger provided.");

        }

        /// Call after construction to start, needed to allow enable_shared_from_this
        void start() {
            // Look up the domain name
            m_resolver.async_resolve(
                m_req.base()[malloy::http::field::host],
                m_port,
                boost::beast::bind_front_handler(
                    &connection::on_resolve,
                    derived().shared_from_this()
                )
            );
        }
    protected:

        std::shared_ptr<spdlog::logger> m_logger;

        void
        send_request()
        {
            // Send the HTTP request to the remote host
            boost::beast::http::async_write(
                derived().stream(),
                m_req,
                boost::beast::bind_front_handler(
                    &connection::on_write,
                    derived().shared_from_this()
                )
            );
        }

    private:
        boost::asio::ip::tcp::resolver m_resolver;
        boost::beast::flat_buffer m_buffer; // (Must persist between reads)
        boost::beast::http::response_parser<boost::beast::http::empty_body> m_parser;
        malloy::http::request<ReqBody> m_req;
        std::optional<callback_t> m_cb;
        std::string m_port;
        Filter m_req_filter;

        [[nodiscard]]
        Derived&
        derived()
        {
            return static_cast<Derived&>(*this);
        }

        void
        on_resolve(const boost::beast::error_code& ec, boost::asio::ip::tcp::resolver::results_type results)
        {
            if (ec) {
                m_logger->error("on_resolve: {}", ec.message());
                report_err(ec);
                //m_err_channel.set_value(ec);
                return;
            }

            // Set a timeout on the operation
            boost::beast::get_lowest_layer(derived().stream()).expires_after(std::chrono::seconds(30));

            // Make the connection on the IP address we get from a lookup
            boost::beast::get_lowest_layer(derived().stream()).async_connect(
                results,
                boost::beast::bind_front_handler(
                    &connection::on_connect,
                    derived().shared_from_this()
                )
            );
        }

        void
        on_connect(const boost::beast::error_code& ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type)
        {
            if (ec) {
                m_logger->error("on_connect: {}", ec.message());
                //m_err_channel.set_value(ec);
                report_err(ec);
                return;
            }

            // Set a timeout on the operation
            boost::beast::get_lowest_layer(derived().stream()).expires_after(std::chrono::seconds(30));

            // Call hook
            derived().hook_connected();
        }

        void
        on_write(const boost::beast::error_code& ec, [[maybe_unused]] const std::size_t bytes_transferred)
        {
            if (ec) {
                m_logger->error("on_write: {}", ec.message());
                //m_err_channel.set_value(ec);
                report_err(ec);
                return;
            }

            // Receive the HTTP response
            boost::beast::http::async_read_header(
                derived().stream(),
                m_buffer,
                m_parser,
                malloy::bind_front_handler(
                    &connection::on_read_header,
                    derived().shared_from_this()
                )
            );

       }
        void on_read_header(malloy::error_code ec, std::size_t) {
            if (ec) {
                m_logger->error("on_read_header: '{}'", ec.message());
                //m_err_channel.set_value(ec);
                report_err(ec);
                return;
            }

            // Pick a body and parse it from the stream
            auto bodies = m_req_filter.body_for(m_parser.get().base());
            std::visit([this](auto&& body) {
                using body_t = std::decay_t<decltype(body)>;
                auto parser = std::make_shared<boost::beast::http::response_parser<body_t>>(std::move(m_parser));
                m_req_filter.setup_body(parser->get().base(), parser->get().body());
                boost::beast::http::async_read(
                    derived().stream(),
                    m_buffer,
                    *parser,
                    [this, parser, me = derived().shared_from_this()](auto ec, auto) {
                    if (ec) {
                        m_logger->error("on_read(): {}", ec.message());
                        //m_err_channel.set_value(ec);
                        report_err(ec);
                        return;
                    }
                    // Notify via callback
                    (*m_cb)(ec, malloy::http::response<body_t>{parser->release()});
                    on_read();
                }
                );
                }, std::move(bodies));
        }

        void
        on_read()
        {
            // Gracefully close the socket
            malloy::error_code ec;
            boost::beast::get_lowest_layer(derived().stream()).socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

            // not_connected happens sometimes so don't bother reporting it.
            if (ec && ec != boost::beast::errc::not_connected) {
                m_logger->error("shutdown: {}", ec.message());
            }
            if (ec) {
                report_err(ec);
            }
        }
        void report_err(error_code ec) {
            tmp::filter_resp_t<Filter> v;
            (*m_cb)(ec, v);
        }
    };

}
