#include "session.hpp"
#include "router.hpp"
#include "../websocket/session.hpp"

#include <spdlog/spdlog.h>

using namespace malloy::server;
using namespace malloy::server::http;

session::session(
    std::shared_ptr<spdlog::logger> logger,
    tcp::socket&& socket,
    std::shared_ptr<router> router,
    std::shared_ptr<const std::string> http_doc_root
) :
    m_logger(std::move(logger)),
    m_stream(std::move(socket)),
    m_router(std::move(router)),
    m_doc_root(std::move(http_doc_root)),
    m_queue(*this)
{
    // Sanity check logger
    if (not m_logger)
        throw std::runtime_error("did not receive a valid logger instance.");

    // Sanity check router
    if (not m_router)
        throw std::runtime_error("did not receive a valid router instance.");
}

void session::run()
{
    m_logger->trace("run()");

    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.
    boost::asio::dispatch(
            m_stream.get_executor(),
            boost::beast::bind_front_handler(
                    &session::do_read,
                    this->shared_from_this()));
}

void session::do_read()
{
    m_logger->trace("do_read()");

    // Construct a new parser for each message
    m_parser.emplace();

    // Apply a reasonable limit to the allowed size
    // of the body in bytes to prevent abuse.
    m_parser->body_limit(100000);

    // Set the timeout.
    m_stream.expires_after(std::chrono::seconds(30));

    // Read a request using the parser-oriented interface
    boost::beast::http::async_read(
            m_stream,
            m_buffer,
            *m_parser,
            boost::beast::bind_front_handler(
                    &session::on_read,
                    shared_from_this()));
}

void session::on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    m_logger->trace("on_read(): bytes read: {}", bytes_transferred);

    // This means they closed the connection
    if (ec == boost::beast::http::error::end_of_stream)
        return do_close();

    // Check for errors
    if (ec) {
        m_logger->error("on_read(): {}", ec.message());
        return;
    }

    // See if it is a WebSocket Upgrade
    if (boost::beast::websocket::is_upgrade(m_parser->get())) {
        m_logger->info("upgrading HTTP session to WS session.");

        // Create a websocket session, transferring ownership
        // of both the socket and the HTTP request.
        auto ws_session = std::make_shared<websocket::session>(m_logger->clone("websocket_session"), m_stream.release_socket());
        ws_session->do_accept(m_parser->release());
        return;
    }

    // Hand off to router
    m_router->handle_request(*m_doc_root, m_parser->release(), m_queue);
}

void session::on_write(bool close, boost::beast::error_code ec, std::size_t bytes_transferred)
{
    m_logger->trace("on_write(): bytes written: {}", bytes_transferred);

    // Check for errors
    if (ec) {
        m_logger->error("on_write(): {}", ec.message());
        return;
    }

    if (close) {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        return do_close();
    }

    // Inform the queue that a write completed
    if (m_queue.on_write()) {
        // Read another request
        do_read();
    }
}

void session::do_close()
{
    m_logger->trace("do_close()");

    // Send a TCP shutdown
    boost::beast::error_code ec;
    m_stream.socket().shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
    spdlog::info("closed HTTP session gracefully.");
}
