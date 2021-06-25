#pragma once

#include "malloy/websocket/types.hpp"
#include "malloy/server/http/connection/connection.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/strand.hpp>
#include <spdlog/logger.h>

#include <future>
#include <queue>
#include <thread>

namespace malloy::server::websocket
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
        using request_generator = malloy::server::http::request_generator<connection<Derived>>;
        using handler_t = std::function<void(request_generator&, connection&)>;

        enum class state
        {
            handshaking,
            active,
            closing,
            closed
        };

        /**
         * The agent string.
         */
        static const std::string agent_string;

        connection(
            std::shared_ptr<spdlog::logger> logger
        ) :
            m_logger(std::move(logger))
        {
            // Sanity check logger
            if (!m_logger)
                throw std::invalid_argument("no valid logger provided.");
        }

        void set_handler(handler_t&& handler)
        {
            m_handler = std::move(handler);
        }

        // Start the asynchronous operation
        template<class Body, class Allocator>
        void
        run(boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req)
        {
            // Update state
            m_state = state::handshaking;

            // Accept the WebSocket upgrade request
            do_accept(std::move(req));
        }

        // ToDo: Test this
        void stop()
        {
            m_state = state::closing;

            boost::asio::dispatch(
                boost::beast::bind_front_handler(
                    &connection::on_close(),
                    derived().shared_from_this()
                )
            );
        }

        /**
         * Send a payload to the client.
         *
         * @param payload The payload to send.
         */
        void send(std::string payload)
        {
            m_logger->trace("send(). payload size: {}", payload.size());

            boost::asio::dispatch(
                boost::asio::bind_executor(
                    derived().stream().get_executor(),
                    [self = derived().shared_from_this(), msg = std::move(payload)]() mutable {
                        self->handle_send(std::move(msg));
                    }
                )
            );
        }

    private:
        enum class sending_state
        {
            idling,
            sending
        };

        enum sending_state m_sending_state = sending_state::idling;

        std::shared_ptr<spdlog::logger> m_logger;
        boost::beast::flat_buffer m_buffer;
        std::queue<std::string> m_tx_queue;
        handler_t m_handler;
        enum state m_state = state::closed;

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
            m_logger->trace("do_accept()");

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
            m_logger->trace("on_accept()");

            // Check for errors
            if (ec) {
                m_logger->error("on_accept(): {}", ec.message());
                return;
            }

            // We're good to go
            m_state = state::active;
            initiate_rx();
            maybe_send_next();
        }

        void
        initiate_rx()
        {
            m_logger->trace("do_read()");

            assert(m_state == state::active);

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
            m_logger->trace("on_read(): bytes read: {}", bytes_transferred);

            // This indicates that the connection was closed
            if (ec == boost::beast::websocket::error::closed) {
                m_logger->info("on_read(): connection was closed.");
                return;
            }

            // Check for errors
            if (ec) {
                m_logger->error("on_read(): {}", ec.message());
                return;
            }

            // Nothing to do if we didn't read anything
            if (bytes_transferred == 0)
                return;

            // Convert to string
            std::string payload = boost::beast::buffers_to_string(m_buffer.cdata());
            if (payload.empty())
                return;

            // Consume the buffer
            m_buffer.consume(m_buffer.size());

            // Handle
            if (m_handler)
                m_handler(payload, [this](const malloy::websocket::payload_t& resp) {
                    send(resp);
                });

            // Do more reading
            initiate_rx();
        }

        void
        handle_send(std::string msg)
        {
            m_logger->trace("handle_send()");

            m_tx_queue.push(std::move(msg));
            maybe_send_next();
        }

        void
        maybe_send_next()
        {
            m_logger->trace("maybe_send_next()");

            if (m_state != state::active)
                return;
            if (m_sending_state == sending_state::sending)
                return;
            if (m_tx_queue.empty())
                return;

            initiate_tx();
        }

        void
        initiate_tx()
        {
            m_logger->trace("initiate_tx()");

            assert(m_sending_state == sending_state::idling);
            assert(!m_tx_queue.empty());

            m_sending_state = sending_state::sending;
            derived().stream().async_write(
                boost::asio::buffer(m_tx_queue.front()),
                [self = derived().shared_from_this()](const boost::beast::error_code& ec, std::size_t) {
                    // we don't care about bytes_transferred
                    self->handle_tx(ec);
                });
        }

        void
        handle_tx(const boost::beast::error_code& ec)
        {
            m_logger->trace("handle_tx()");

            if (ec) {
                m_logger->error("failed to send message: {}", ec.message());
                return;
            }

            m_tx_queue.pop();
            m_sending_state = sending_state::idling;
            maybe_send_next();
        }

        void
        on_close()
        {
            m_logger->trace("on_close()");

            m_state = state::closed;
        }
    };

    template<typename Derived>
    const std::string connection<Derived>::agent_string = std::string(BOOST_BEAST_VERSION_STRING) + " malloy";

}
