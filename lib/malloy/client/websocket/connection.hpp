#pragma once

#include "../../websocket/types.hpp"

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <spdlog/logger.h>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <queue>

namespace malloy::client::websocket
{

    template<class Derived>
    class connection
    {
    public:
        enum class state
        {
            handshaking,
            active,
            closing,
            closed
        };

        explicit
        connection(std::shared_ptr<spdlog::logger> logger, boost::asio::io_context& ioc, malloy::websocket::handler_t&& handler) :
            m_logger(std::move(logger)),
            m_resolver(boost::asio::make_strand(ioc)),
            m_handler(std::move(handler))
        {
            // Sanity check
            if (!m_logger)
                throw std::invalid_argument("no valid logger provided.");
        }

        // Start the asynchronous operation
        void
        connect(std::string host, const std::string& port, std::string endpoint)
        {
            m_logger->trace("connect({}, {}, {})", host, port, endpoint);

            // Save these for later
            m_host = std::move(host);
            m_endpoint = std::move(endpoint);

            // Look up the domain name
            m_resolver.async_resolve(
                m_host,
                port,
                boost::beast::bind_front_handler(
                    &connection::on_resolve,
                    derived().shared_from_this()
                )
            );
        }

        void
        disconnect()
        {
            m_logger->trace("close()");

            m_state = state::closing;

            // Close the WebSocket connection
            derived().stream().async_close(
                boost::beast::websocket::close_code::normal,
                boost::beast::bind_front_handler(
                    &connection::on_close,
                    derived().shared_from_this()
                )
            );
        }

        void send(const std::string& payload)
        {
            m_logger->trace("send()");

            // Sanity check
            if (payload.empty())
                return;

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

        std::shared_ptr<spdlog::logger> m_logger;
        boost::asio::ip::tcp::resolver m_resolver;
        boost::beast::flat_buffer m_buffer;
        std::string m_host;
        std::string m_endpoint;
        malloy::websocket::handler_t m_handler;
        std::queue<std::string> m_tx_queue;
        enum state m_state = state::closed;
        enum sending_state m_sending_state = sending_state::idling;

        [[nodiscard]]
        Derived&
        derived()
        {
            return static_cast<Derived&>(*this);
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
                }
            );
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

        void initiate_rx()
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
        on_resolve(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results)
        {
            m_logger->trace("on_resolve()");

            if (ec) {
                m_logger->error("on_resolve(): {}", ec.message());
                return;
            }

            // Set the timeout for the operation
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
        on_connect(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type ep)
        {
            m_logger->trace("on_connect()");

            if (ec) {
                m_logger->error("on_connect(): {}", ec.message());
                return;
            }

            // Alias stream for readability
            auto& stream = derived().stream();

            // Turn off the timeout on the tcp_stream, because
            // the websocket stream has its own timeout system.
            boost::beast::get_lowest_layer(stream).expires_never();

            // Set suggested timeout settings for the websocket
            stream.set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::client));

            // Set a decorator to change the User-Agent of the handshake
            stream.set_option(
                boost::beast::websocket::stream_base::decorator(
                    [](boost::beast::websocket::request_type& req)
                    {
                        req.set(boost::beast::http::field::user_agent, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-async");
                    }
                )
            );

            // Update the m_host string. This will provide the value of the
            // Host HTTP header during the WebSocket handshake.
            // See https://tools.ietf.org/html/rfc7230#section-5.4
            m_host += ':' + std::to_string(ep.port());

            // Perform the websocket handshake
            derived().stream().async_handshake(
                m_host,
                m_endpoint,
                boost::beast::bind_front_handler(
                    &connection::on_handshake,
                    derived().shared_from_this()
                )
            );
        }

        void
        on_handshake(boost::beast::error_code ec)
        {
            m_logger->trace("on_handshake()");

            if (ec) {
                m_logger->error("on_handshake(): {}", ec.message());
                return;
            }

            // We're good to go
            m_state = state::active;
            initiate_rx();
            maybe_send_next();
        }

        void
        on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            m_logger->trace("on_read()");

            if (ec) {
                m_logger->error("on_read(): {}", ec.message());
                return;
            }

            // Convert to string
            std::string payload = boost::beast::buffers_to_string(m_buffer.cdata());
            if (payload.empty())
                return;

            // Handle
            if (m_handler)
                m_handler(payload, [this](const std::string& payload){
                    send(payload);
                });

            // Consume the buffer
            m_buffer.consume(m_buffer.size());

            // Read more
            initiate_rx();
        }

        void
        on_close(boost::beast::error_code ec)
        {
            m_logger->trace("on_close()");

            if (ec) {
                m_logger->error("on_close(): {}", ec.message());
                return;
            }

            // If we get here then the connection is closed gracefully
            m_state = state::closed;
        }
    };

}
