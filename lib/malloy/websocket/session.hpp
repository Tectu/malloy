#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/strand.hpp>

#include <algorithm>
#include <cstdlib>
#include <memory>

namespace spdlog
{
    class logger;
}

namespace malloy::websocket::server
{
    class controller;

    /**
     * A websocket session.
     */
    class session :
        public std::enable_shared_from_this<session>
    {
    public:

        /**
         * The agent string.
         */
        static const std::string agent_string;

        // Take ownership of the socket
        session(std::shared_ptr<spdlog::logger> logger, boost::asio::ip::tcp::socket&& socket);

        // Start the asynchronous accept operation
        template<class Body, class Allocator>
        void do_accept(boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req)
        {
            // Set suggested timeout settings for the websocket
            m_websocket.set_option(
                boost::beast::websocket::stream_base::timeout::suggested(
                    boost::beast::role_type::server));

            // Set a decorator to change the Server of the handshake
            m_websocket.set_option(boost::beast::websocket::stream_base::decorator(
                [](boost::beast::websocket::response_type& res)
                {
                    res.set(boost::beast::http::field::server, agent_string);
                }));

            // Accept the websocket handshake
            m_websocket.async_accept(
                req,
                boost::beast::bind_front_handler(
                    &session::on_accept,
                    shared_from_this()));
        }

        void write(std::string&& payload);

    private:
        std::shared_ptr<spdlog::logger> m_logger;
        boost::beast::websocket::stream<boost::beast::tcp_stream> m_websocket;
        boost::beast::flat_buffer m_buffer;

        void on_accept(const boost::beast::error_code ec);
        void do_read();
        void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);
        void on_write(boost::beast::error_code ec, std::size_t bytes_transferred);
    };

}
