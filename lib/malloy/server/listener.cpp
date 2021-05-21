#include "listener.hpp"

#include "http/connection/connection_detector.hpp"

#include <boost/asio/strand.hpp>
#include <spdlog/logger.h>

using namespace malloy;
using namespace malloy::server;

listener::listener(
    std::shared_ptr<spdlog::logger> logger,
    boost::asio::io_context& ioc,
    std::shared_ptr<boost::asio::ssl::context> tls_ctx,
    const boost::asio::ip::tcp::endpoint& endpoint,
    std::shared_ptr<server::router> router,
    std::shared_ptr<const std::filesystem::path> http_doc_root,
    websocket::handler_type websocket_handler
) :
    m_logger(std::move(logger)),
    m_io_ctx(ioc),
    m_tls_ctx(std::move(tls_ctx)),
    m_acceptor(boost::asio::make_strand(ioc)),
    m_router(std::move(router)),
    m_doc_root(std::move(http_doc_root)),
    m_websocket_handler(std::move(websocket_handler))
{
    boost::beast::error_code ec;

    // Sanity check on logger
    if (not m_logger)
        throw std::runtime_error("did not receive a valid logger instance.");

    // Open the acceptor
    m_acceptor.open(endpoint.protocol(), ec);
    if (ec) {
        m_logger->critical("listener::listener(): could not open acceptor: {}", ec.message());
        return;
    }

    // Allow address reuse
    m_acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
    if (ec) {
        m_logger->critical("listener::listener(): could not set option to allow address reuse: {}", ec.message());
        return;
    }

    // Bind to the server address
    m_acceptor.bind(endpoint, ec);
    if (ec) {
        m_logger->critical("could not bind to end-point: {}", ec.message());
        return;
    }

    // Start listening for connections
    m_acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec) {
        m_logger->critical("could not start licensing: {}", ec.message());
        return;
    }
}

// Start accepting incoming connections
void listener::run()
{
    m_logger->trace("listener::run()");

    // ToDo: This can most likely be replaced to a call to this->do_accept()

    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this connection. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.
    boost::asio::dispatch(
        m_acceptor.get_executor(),
        boost::beast::bind_front_handler(
            &listener::do_accept,
            this->shared_from_this())
    );
}

void listener::do_accept()
{
    m_logger->trace("listener::do_accept()");

    // The new connection gets its own strand
    m_acceptor.async_accept(
        boost::asio::make_strand(m_io_ctx),
        boost::beast::bind_front_handler(
            &listener::on_accept,
            shared_from_this())
    );
}

void listener::on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket)
{
    m_logger->trace("listener::on_accept()");

    // Check for errors
    if (ec) {
        m_logger->error("listener::on_accept(): {}", ec.message());
        return;
    }

    // Log
    m_logger->info(
        "accepting incoming connection from {}:{}",
        socket.remote_endpoint().address().to_string(),
        socket.remote_endpoint().port()
    );

    // Create the http detector connection
    // This will automatically spawn the correct type of connection (eg. plain or TLS).
    auto connection = std::make_shared<http::connection_detector>(
        m_logger->clone("http_connection"),
        std::move(socket),
        m_tls_ctx,
        m_doc_root,
        m_router,
        m_websocket_handler
    );

    // Run the HTTP connection
    connection->run();

    // Accept another connection
    do_accept();
}
