#pragma once

#include "../../http/request.hpp"
#include "../../http/response.hpp"

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <spdlog/logger.h>

namespace malloy::client::http
{

    /**
     * An HTTP connection.
     *
     * @tparam Derived The type inheriting from this class.
     */
    template<class Derived>
    class connection
    {
    public:
        using callback_t = std::function<void(malloy::http::response&&)>;

        connection(std::shared_ptr<spdlog::logger> logger, boost::asio::io_context& io_ctx) :
            m_logger(std::move(logger)),
            m_resolver(boost::asio::make_strand(io_ctx))
        {
            // Sanity check
            if (!m_logger)
                throw std::invalid_argument("no valid logger provided.");
        }

        // Start the asynchronous operation
        void
        run(
            char const* port,
            malloy::http::request req,
            callback_t&& cb
        )
        {
            m_req = std::move(req);
            m_cb = std::move(cb);
            if (!m_cb)
                return m_logger->error("no callback set. ignoring.");

            // Look up the domain name
            m_resolver.async_resolve(
                m_req.base()[malloy::http::field::host],
                port,
                boost::beast::bind_front_handler(
                    &connection::on_resolve,
                    derived().shared_from_this()
                )
            );
        }

    private:
        std::shared_ptr<spdlog::logger> m_logger;
        boost::asio::ip::tcp::resolver m_resolver;
        boost::beast::flat_buffer m_buffer; // (Must persist between reads)
        malloy::http::request m_req;
        malloy::http::response m_resp;
        callback_t m_cb;

        [[nodiscard]]
        Derived&
        derived()
        {
            return static_cast<Derived&>(*this);
        }

        void
        on_resolve(const boost::beast::error_code& ec, boost::asio::ip::tcp::resolver::results_type results)
        {
            if (ec)
                return m_logger->error("on_resolve: {}", ec.message());

            // Set a timeout on the operation
            derived().stream().expires_after(std::chrono::seconds(30));

            // Make the connection on the IP address we get from a lookup
            derived().stream().async_connect(
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
            if (ec)
                return m_logger->error("on_connect: {}", ec.message());

            // Set a timeout on the operation
            derived().stream().expires_after(std::chrono::seconds(30));

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

        void
        on_write(const boost::beast::error_code& ec, [[maybe_unused]] const std::size_t bytes_transferred)
        {
            if (ec)
                return m_logger->error("on_write: {}", ec.message());

            // Receive the HTTP response
            boost::beast::http::async_read(
                derived().stream(),
                m_buffer,
                m_resp,
                boost::beast::bind_front_handler(
                    &connection::on_read,
                    derived().shared_from_this()
                )
            );
        }

        void
        on_read(boost::beast::error_code ec, [[maybe_unused]] std::size_t bytes_transferred)
        {
            if (ec)
                return m_logger->error("on_read(): {}", ec.message());

            // Notify via callback
            if (m_cb)
                m_cb(std::move(m_resp));

            // Gracefully close the socket
            derived().stream().socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

            // not_connected happens sometimes so don't bother reporting it.
            if (ec && ec != boost::beast::errc::not_connected)
                return m_logger->error("shutdown: {}", ec.message());

            // If we get here then the connection is closed gracefully
        }
    };

}
