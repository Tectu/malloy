#include "session.hpp"

#include <spdlog/spdlog.h>

using namespace malloy::server::websocket;

// Agent string
const std::string session::agent_string = std::string(BOOST_BEAST_VERSION_STRING) + " carava-server";

session::session(
    std::shared_ptr<spdlog::logger> logger,
    boost::asio::ip::tcp::socket&& socket
) :
    m_websocket(std::move(socket))
{
    // Sanity check logger
    if (not m_logger)
        throw std::runtime_error("did not receive a valid logger instance.");

    // We operate in binary mode
    m_websocket.binary(true);
}

void session::write(std::string&& payload)
{
    m_logger->trace("write(). payload size: {}", payload.size());

    // Sanity check
    if (payload.empty())
        return;

    // Issue asynchronous write
    m_websocket.async_write(
        boost::asio::buffer(std::move(payload)),
        boost::beast::bind_front_handler(&session::on_write, shared_from_this())
    );
}

void session::on_accept(const boost::beast::error_code ec)
{
    m_logger->trace("on_accept()");

    // Check for errors
    if (ec) {
        m_logger->error("on_accept(): {}", ec.message());
        return;
    }

    // Read a message
    do_read();
}

void session::do_read()
{
    m_logger->trace("do_read()");

    // Read a message into our buffer
    m_websocket.async_read(m_buffer, boost::beast::bind_front_handler(&session::on_read, shared_from_this()));
}

void session::on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    m_logger->trace("on_read(): bytes read: {}", bytes_transferred);

    // This indicates that the session was closed
    if (ec == boost::beast::websocket::error::closed) {
        m_logger->info("on_read(): session was closed.");
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

    // Do more reading
    do_read();
}

void session::on_write(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    m_logger->trace("on_write(): bytes written: {}", bytes_transferred);

    // Clear the buffer
    m_buffer.consume(m_buffer.size());

    // Check for errors
    if (ec) {
        m_logger->error("on_write(): {}", ec.message());
        return;
    }
}
