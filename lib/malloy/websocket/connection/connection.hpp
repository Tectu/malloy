#pragma once

#include "../types.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/strand.hpp>
#include <spdlog/logger.h>

namespace malloy::websocket::server
{

    /**
     * Base class for a websocket connection/session.
     *
     * This uses CRTP to make the code reusable for different implementations (eg. plain vs. TLS).
     *
     * @tparam Derived
     */
    template<class Derived>
    class connection
    {
    public:
        /**
         * The agent string.
         */
        static const std::string agent_string;

        connection(
            std::shared_ptr<spdlog::logger> logger,
            handler_type handler
        ) :
            m_logger(std::move(logger)),
            m_handler(std::move(handler))
        {
            // Sanity check logger
            if (not m_logger)
                throw std::invalid_argument("no valid logger provided.");
        }

        // Start the asynchronous operation
        template<class Body, class Allocator>
        void
        run(boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req)
        {
            // Accept the WebSocket upgrade request
            do_accept(std::move(req));
        }

        /**
         * Send a payload to the client.
         *
         * @param payload The payload to send.
         */
        void write(std::string&& payload)
        {
            m_logger->trace("write(). payload size: {}", payload.size());

            // Sanity check
            if (payload.empty())
                return;

            // Issue asynchronous write
            derived().stream().async_write(
                boost::asio::buffer(payload),
                boost::beast::bind_front_handler(&connection::on_write, derived().shared_from_this())
            );
        }

    private:
        std::shared_ptr<spdlog::logger> m_logger;
        handler_type m_handler;
        boost::beast::flat_buffer m_buffer;

        [[nodiscard]]
        Derived&
        derived()
        {
            return static_cast<Derived&>(*this);
        }

        // Start the asynchronous operation
        template<class Body, class Allocator>
        void
        do_accept(boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req)
        {
            // Set suggested timeout settings for the websocket
            derived().stream().set_option(
                boost::beast::websocket::stream_base::timeout::suggested(
                    boost::beast::role_type::server
                )
            );

            // Set a decorator to change the Server of the handshake
            derived().stream().set_option(
                boost::beast::websocket::stream_base::decorator(
                    [](boost::beast::websocket::response_type& res)
                    {
                        res.set(boost::beast::http::field::server, agent_string);
                    }
                )
            );

            // Accept the websocket handshake
            derived().stream().async_accept(
                req,
                boost::beast::bind_front_handler(
                    &connection::on_accept,
                    derived().shared_from_this()
                )
            );
        }

        void
        on_accept(boost::beast::error_code ec)
        {
            // Check for errors
            if (ec) {
                m_logger->error("on_accept(): {}", ec.message());
                return;
            }

            // Read a message
            do_read();
        }

        void
        do_read()
        {
            // Read a message into our buffer
            derived().stream().async_read(
                m_buffer,
                boost::beast::bind_front_handler(
                    &connection::on_read,
                    derived().shared_from_this()
                )
            );
        }

        void
        on_read(
            boost::beast::error_code ec,
            std::size_t bytes_transferred
        )
        {
            // This indicates that the connection was closed
            if (ec == boost::beast::websocket::error::closed) {
                m_logger->info("on_read(): connection was closed.");
                return;
            }

            // Check for errors
            if (ec) {
                m_logger->error("on_read(): {}", ec.message());
            }

            // Nothing to do if we didn't read anything
            if (bytes_transferred == 0)
                return;

            // Convert to string
            std::string payload = boost::beast::buffers_to_string(m_buffer.cdata());
            if (payload.empty())
                return;

            // Handle the payload
            if (m_handler) {
                try {
                    m_handler(payload, std::bind(&connection::write, this, std::placeholders::_1));
                }
                catch (const std::exception& e) {
                    m_logger->critical("reader exception: {}", e.what());
                }
                catch (...) {
                    m_logger->critical("unknown reader exception.");
                }
            }

            // Consume the buffer
            m_buffer.consume(m_buffer.size());

            // Do more reading
            do_read();
        }

        void
        on_write(
            boost::beast::error_code ec,
            [[maybe_unused]] std::size_t bytes_transferred
        )
        {
            // Clear the buffer
            m_buffer.consume(m_buffer.size());

            // Check for errors
            if (ec) {
                m_logger->error("on_write(): {}", ec.message());
                return;
            }
        }
    };

    template<typename Derived>
    const std::string connection<Derived>::agent_string = std::string(BOOST_BEAST_VERSION_STRING) + " malloy";

}
