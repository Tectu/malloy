#pragma once

#include "connection.hpp"

namespace malloy::http::server
{

    class connection_plain :
        public connection<connection_plain>,
        public std::enable_shared_from_this<connection_plain>
    {
        friend connection;

    public:
        // Create the session
        connection_plain(
            std::shared_ptr<spdlog::logger> logger,
            boost::asio::ip::tcp::socket&& socket,
            boost::beast::flat_buffer buffer,
            std::shared_ptr<const std::filesystem::path> doc_root,
            std::shared_ptr<router> router,
            malloy::websocket::handler_type websocket_handler
        ) :
            connection<connection_plain>(
                std::move(logger),
                std::move(buffer),
                std::move(router),
                std::move(doc_root),
                std::move(websocket_handler)
            ),
            m_stream(std::move(socket))
        {
        }

        // Called by the base class
        [[nodiscard]]
        boost::beast::tcp_stream&
        stream()
        {
            return m_stream;
        }

        /**
         * Release the stream.
         *
         * @return The stream.
         */
        [[nodiscard]]
        boost::beast::tcp_stream
        release_stream()
        {
            return std::move(m_stream);
        }

        /**
         * Start the asynchronous operation
         */
        void
        run()
        {
            // We need to be executing within a strand to perform async operations
            // on the I/O objects in this session. Although not strictly necessary
            // for single-threaded contexts, this example code is written to be
            // thread-safe by default.
            boost::asio::dispatch(m_stream.get_executor(),
                boost::beast::bind_front_handler(
                    &connection::do_read,
                    shared_from_this()
                )
            );
        }

        /**
         * Close the connection.
         */
        void
        do_close()
        {
            // Send a TCP shutdown
            boost::beast::error_code ec;
            m_stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);

            // At this point the connection is closed gracefully
        }

    private:
        boost::beast::tcp_stream m_stream;
    };

}
