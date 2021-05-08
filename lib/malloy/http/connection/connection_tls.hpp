#pragma once

#include "connection.hpp"

#include <boost/beast/ssl/ssl_stream.hpp>

namespace malloy::http::server
{

    /**
     * A TLS connection.
     */
    class connection_tls :
        public connection<connection_tls>,
        public std::enable_shared_from_this<connection_tls>
    {

    public:
        connection_tls(
            std::shared_ptr<spdlog::logger> logger,
            boost::asio::ip::tcp::socket&& socket,
            boost::asio::ssl::context& ctx,
            boost::beast::flat_buffer buffer,
            std::shared_ptr<const std::filesystem::path> const& doc_root
        ) :
            connection<connection_tls>(
                logger,
                std::move(buffer),
                nullptr,    // ToDo <---------------------------------
                doc_root,
                nullptr     // ToDo <---------------------------------
            ),
            m_stream(std::move(socket), ctx)
        {
        }

        // Called by the base class
        boost::beast::ssl_stream<boost::beast::tcp_stream>&
        stream()
        {
            return m_stream;
        }

        // Start the asynchronous operation
        void run()
        {
            auto self = shared_from_this();

            // We need to be executing within a strand to perform async operations
            // on the I/O objects in this session.
            boost::asio::dispatch(m_stream.get_executor(), [self](){
                // Set the timeout.
                boost::beast::get_lowest_layer(self->m_stream).expires_after(std::chrono::seconds(30));

                // Perform the SSL handshake
                // Note, this is the buffered version of the handshake.
                self->m_stream.async_handshake(
                    boost::asio::ssl::stream_base::server,
                    self->m_buffer.data(),
                    boost::beast::bind_front_handler(
                        &connection_tls::on_handshake,
                        self
                    )
                );
            });
        }

        void on_handshake(boost::beast::error_code ec, const std::size_t bytes_used)
        {
            if (ec) {
                // ToDO
                //return fail(ec, "handshake");
                return;
            }

            // Consume the portion of the buffer used by the handshake
            m_buffer.consume(bytes_used);

            do_read();
        }

        void do_close()
        {
            // Set the timeout.
            boost::beast::get_lowest_layer(m_stream).expires_after(std::chrono::seconds(30));

            // Perform the SSL shutdown
            m_stream.async_shutdown(
                boost::beast::bind_front_handler(
                    &connection_tls::on_shutdown,
                    shared_from_this()
                )
            );
        }

        void on_shutdown([[maybe_unused]] boost::beast::error_code ec)
        {
            // At this point the connection is closed gracefully
        }

    private:
        boost::beast::ssl_stream<boost::beast::tcp_stream> m_stream;
    };

}
