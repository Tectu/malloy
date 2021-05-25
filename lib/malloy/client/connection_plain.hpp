#pragma once

#include "../websocket/types.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <spdlog/logger.h>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <queue>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

namespace malloy::client
{

    // Report a failure
    void
    fail(beast::error_code ec, char const* what)
    {
        std::cerr << what << ": " << ec.message() << "\n";
    }

    class connection_plain :
        public std::enable_shared_from_this<connection_plain>
    {
    public:
        enum class state
        {
            handshaking,
            active,
            closing,
            closed
        };

        // Resolver and socket require an io_context
        explicit
        connection_plain(std::shared_ptr<spdlog::logger> logger, boost::asio::io_context& ioc, malloy::websocket::handler_t&& handler) :
            m_logger(std::move(logger)),
            m_resolver(boost::asio::make_strand(ioc)),
            m_stream(boost::asio::make_strand(ioc)),
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
                beast::bind_front_handler(
                    &connection_plain::on_resolve,
                    shared_from_this()
                )
            );
        }

        void
        disconnect()
        {
            m_logger->trace("close()");

            m_state = state::closing;

            // Close the WebSocket connection
            m_stream.async_close(boost::beast::websocket::close_code::normal,
                beast::bind_front_handler(
                    &connection_plain::on_close,
                    shared_from_this()
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
                    m_stream.get_executor(),
                    [self = shared_from_this(), msg = std::move(payload)]() mutable {
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
        boost::beast::websocket::stream<beast::tcp_stream> m_stream;
        beast::flat_buffer m_buffer;
        std::string m_host;
        std::string m_endpoint;
        malloy::websocket::handler_t m_handler;
        std::queue<std::string> m_tx_queue;
        enum state m_state = state::closed;
        enum sending_state m_sending_state = sending_state::idling;

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
            m_stream.async_write(
                boost::asio::buffer(m_tx_queue.front()),
                [self = shared_from_this()](const boost::beast::error_code& ec, std::size_t) {
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
            m_stream.async_read(
                m_buffer,
                beast::bind_front_handler(
                    &connection_plain::on_read,
                    shared_from_this()
                )
            );
        }

        void
        on_resolve(beast::error_code ec, tcp::resolver::results_type results)
        {
            m_logger->trace("on_resolve()");

            if (ec)
                return fail(ec, "resolve");

            // Set the timeout for the operation
            beast::get_lowest_layer(m_stream).expires_after(std::chrono::seconds(30));

            // Make the connection on the IP address we get from a lookup
            beast::get_lowest_layer(m_stream).async_connect(
                results,
                beast::bind_front_handler(
                    &connection_plain::on_connect,
                    shared_from_this()
                )
            );
        }

        void
        on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
        {
            m_logger->trace("on_connect()");

            if (ec)
                return fail(ec, "connect");

            // Turn off the timeout on the tcp_stream, because
            // the websocket stream has its own timeout system.
            beast::get_lowest_layer(m_stream).expires_never();

            // Set suggested timeout settings for the websocket
            m_stream.set_option(boost::beast::websocket::stream_base::timeout::suggested(beast::role_type::client));

            // Set a decorator to change the User-Agent of the handshake
            m_stream.set_option(
                boost::beast::websocket::stream_base::decorator(
                    [](boost::beast::websocket::request_type& req)
                    {
                        req.set(http::field::user_agent, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-async");
                    }
                )
            );

            // Update the m_host string. This will provide the value of the
            // Host HTTP header during the WebSocket handshake.
            // See https://tools.ietf.org/html/rfc7230#section-5.4
            m_host += ':' + std::to_string(ep.port());

            // Perform the websocket handshake
            m_stream.async_handshake(
                m_host,
                m_endpoint,
                beast::bind_front_handler(
                    &connection_plain::on_handshake,
                    shared_from_this()
                )
            );
        }

        void
        on_handshake(beast::error_code ec)
        {
            m_logger->trace("on_handshake()");

            if (ec)
                return fail(ec, "handshake");

            // We're good to go
            m_state = state::active;
            initiate_rx();
            maybe_send_next();
        }

        void
        on_write(beast::error_code ec, std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            m_logger->trace("on_write()");

            if (ec)
                return fail(ec, "write");
        }

        void
        on_read(beast::error_code ec, std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            m_logger->trace("on_read()");

            if(ec)
                return fail(ec, "read");

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
        on_close(beast::error_code ec)
        {
            m_logger->trace("on_close()");

            if (ec)
                return fail(ec, "close");

            // If we get here then the connection is closed gracefully
        }
    };

}
