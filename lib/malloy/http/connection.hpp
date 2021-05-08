#pragma once

#include "../websocket/types.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace spdlog
{
    class logger;
}

namespace malloy::http::server
{
    class router;

    /**
     * An HTTP connection.
     *
     * @brief Handles an HTTP server connection.
     */
    class connection:
        public std::enable_shared_from_this<connection>
    {
        // This is the C++11 equivalent of a generic lambda.
        // The function object is used to send an HTTP message.
        // ToDo: This is a C++20 library, use templated lambdas.
        struct send_lambda
        {
            connection& m_self;

            explicit
            send_lambda(connection & self) :
                m_self(self)
            {
            }

            template<bool isRequest, class Body, class Fields>
            void
            operator()(boost::beast::http::message<isRequest, Body, Fields>&& msg) const
            {
                // The lifetime of the message has to extend
                // for the duration of the async operation so
                // we use a shared_ptr to manage it.
                auto sp = std::make_shared<boost::beast::http::message<isRequest, Body, Fields>>(std::move(msg));

                // Store a type-erased version of the shared
                // pointer in the class to keep it alive.
                m_self.m_response = sp;

                // Write the response
                boost::beast::http::async_write(
                    m_self.m_stream,
                    *sp,
                    boost::beast::bind_front_handler(
                        &connection::on_write,
                        m_self.shared_from_this(),
                        sp->need_eof()
                    )
                );
            }
        };

    public:
        /**
         * Session configuration structure.
         */
        struct config
        {
            std::uint64_t request_body_limit = 10 * 10e6;   ///< The maximum allowed body request size in bytes.
        };

        /**
         * The connection configuration.
         */
        struct config cfg;

        /**
         * Constructor
         *
         * @param logger
         * @param socket
         * @param router
         * @param http_doc_root
         * @param websocket_handler
         */
        connection(
            std::shared_ptr<spdlog::logger> logger,
            boost::asio::ip::tcp::socket&& socket,
            std::shared_ptr<class router> router,
            std::shared_ptr<const std::filesystem::path> http_doc_root,
            malloy::websocket::handler_type websocket_handler
        );

        /**
         * Start the connection.
         */
        void run();

    private:
        std::shared_ptr<spdlog::logger> m_logger;
        boost::beast::tcp_stream m_stream;
        boost::beast::flat_buffer m_buffer;
        std::shared_ptr<const std::filesystem::path> m_doc_root;
        std::shared_ptr<router> m_router;
        malloy::websocket::handler_type m_websocket_handler;
        std::shared_ptr<void> m_response;
        send_lambda m_send;

        // The parser is stored in an optional container so we can
        // construct it from scratch it at the beginning of each new message.
        boost::optional<boost::beast::http::request_parser<boost::beast::http::string_body>> m_parser;

        void do_read();
        void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);
        void on_write(bool close, boost::beast::error_code ec, std::size_t bytes_transferred);
        void do_close();
    };

}
