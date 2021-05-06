#include "connection.hpp"

#include <spdlog/spdlog.h>

using namespace malloy::websocket::server;

// Agent string
const std::string connection::agent_string = std::string(BOOST_BEAST_VERSION_STRING) + " malloy-server";

connection::connection(
    std::shared_ptr<spdlog::logger> logger,
    boost::asio::ip::tcp::socket&& socket,
    malloy::websocket::handler_type handler
) :
    m_logger(std::move(logger)),
    m_websocket(std::move(socket)),
    m_handler(std::move(handler))
{
    // Sanity check logger
    if (not m_logger)
        throw std::runtime_error("did not receive a valid logger instance.");

    // We operate in binary mode
    m_websocket.binary(true);
}

void connection::write(std::string&& payload)
{
    m_logger->trace("write(). payload size: {}", payload.size());

    // Sanity check
    if (payload.empty())
        return;

    // Issue asynchronous write
    m_websocket.async_write(
        boost::asio::buffer(payload),
        boost::beast::bind_front_handler(&connection::on_write, shared_from_this())
    );
}

void connection::on_accept(const boost::beast::error_code ec)
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

void connection::do_read()
{
    m_logger->trace("do_read()");

    // Read a message into our buffer
    m_websocket.async_read(m_buffer, boost::beast::bind_front_handler(&connection::on_read, shared_from_this()));
}

void connection::on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
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

    // Handle the payload
    if (m_handler) {
        try {
            m_handler(payload, std::bind(&connection::write, this, std::placeholders::_1));
        }
        catch (const std::exception& e) {
            m_logger->critical("reader exception: {}", e.what());
        }
        catch (...) {
            m_logger->critical("unknown reader exception.");
        }
    }

    // Consume the buffer
    m_buffer.consume(m_buffer.size());

    // Do more reading
    do_read();
}

void connection::on_write(boost::beast::error_code ec, std::size_t bytes_transferred)
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
