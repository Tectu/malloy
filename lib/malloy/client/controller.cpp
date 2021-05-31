#include "controller.hpp"

#if MALLOY_FEATURE_TLS
    #include <boost/beast/ssl.hpp>
    #include <boost/certify/extensions.hpp>
    #include <boost/certify/https_verification.hpp>
#endif

using namespace malloy::client;

void controller::test_tls()
{
    /*
    std::string host = "127.0.0.1";
    std::string port = "8080";
    std::string endpoint = "/echo";
    std::string text = "Hello malloy client [tls]";

    // The io_context is required for all I/O
    boost::asio::io_context ioc;

    // The SSL context is required, and holds certificates
    boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};
    ctx.set_verify_mode(boost::asio::ssl::verify_peer | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
    ctx.set_default_verify_paths();
    boost::certify::enable_native_https_server_verification(ctx);

    // Launch the asynchronous operation
    std::make_shared<connection_tls>(ioc, ctx)->run(host, port, endpoint, text);

    // Run the I/O service. The call will return when
    // the socket is closed.
    ioc.run();
     */
}
