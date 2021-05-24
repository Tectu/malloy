#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
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
        boost::asio::ip::tcp::resolver m_resolver;
        websocket::stream<beast::tcp_stream> m_stream;
        beast::flat_buffer m_buffer;
        std::string m_host;
        std::string m_text;

    public:
        // Resolver and socket require an io_context
        explicit
        connection_plain(boost::asio::io_context& ioc) :
            m_resolver(net::make_strand(ioc)),
            m_stream(net::make_strand(ioc))
        {
        }

        // Start the asynchronous operation
        void
        run(char const* host, char const* port, char const* text)
        {
            // Save these for later
            m_host = host;
            m_text = text;

            // Look up the domain name
            m_resolver.async_resolve(
                host,
                port,
                beast::bind_front_handler(
                    &connection_plain::on_resolve,
                    shared_from_this()
                )
            );
        }

        void
        on_resolve(beast::error_code ec, tcp::resolver::results_type results)
        {
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
            if (ec)
                return fail(ec, "connect");

            // Turn off the timeout on the tcp_stream, because
            // the websocket stream has its own timeout system.
            beast::get_lowest_layer(m_stream).expires_never();

            // Set suggested timeout settings for the websocket
            m_stream.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));

            // Set a decorator to change the User-Agent of the handshake
            m_stream.set_option(
                websocket::stream_base::decorator(
                    [](websocket::request_type& req)
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
            m_stream.async_handshake(m_host, "/",
                beast::bind_front_handler(
                    &connection_plain::on_handshake,
                    shared_from_this()
                )
            );
        }

        void
        on_handshake(beast::error_code ec)
        {
            if (ec)
                return fail(ec, "handshake");

            // Send the message
            m_stream.async_write(
                net::buffer(m_text),
                beast::bind_front_handler(
                    &connection_plain::on_write,
                    shared_from_this()
                )
            );
        }

        void
        on_write(beast::error_code ec, std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            if (ec)
                return fail(ec, "write");

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
        on_read(beast::error_code ec, std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            if(ec)
                return fail(ec, "read");

            // Close the WebSocket connection
            m_stream.async_close(websocket::close_code::normal,
                beast::bind_front_handler(
                    &connection_plain::on_close,
                    shared_from_this()));
        }

        void
        on_close(beast::error_code ec)
        {
            if (ec)
                return fail(ec, "close");

            // If we get here then the connection is closed gracefully

            // The make_printable() function helps print a ConstBufferSequence
            std::cout << beast::make_printable(m_buffer.data()) << std::endl;
        }
    };

}
