#include "manager.hpp"
#include "../utils.hpp"

#include <boost/asio/ssl/context.hpp>
#include <openssl/ssl.h>

using namespace malloy::tls;

std::shared_ptr<boost::asio::ssl::context> manager::make_context(
    const std::filesystem::path& cert_path,
    const std::filesystem::path& key_path
)
{
    // Sanity checks
    if (not std::filesystem::is_regular_file(cert_path))
        return { };
    if (not std::filesystem::is_regular_file(key_path))
        return { };

    // Load cert
    const std::string& cert = malloy::file_contents(cert_path);
    if (cert.empty())
        return { };

    // Load key
    const std::string& key = malloy::file_contents(key_path);
    if (key.empty())
        return { };

    // Create the context
    auto ctx = std::make_shared<boost::asio::ssl::context>( boost::asio::ssl::context::tls_server );

    // Options
    ctx->set_options(static_cast<long>(
            boost::asio::ssl::context::default_workarounds |
            boost::asio::ssl::context::no_sslv2 |
            boost::asio::ssl::context::no_sslv3 |
            boost::asio::ssl::context::no_tlsv1 |
            boost::asio::ssl::context::no_tlsv1_1 |
            boost::asio::ssl::context::single_dh_use |
            SSL_OP_CIPHER_SERVER_PREFERENCE
        )
    );

    // Setup cert & key
    ctx->use_certificate_chain(boost::asio::buffer(cert));
    ctx->use_private_key(boost::asio::buffer(key), boost::asio::ssl::context::pem);

    return ctx;
}
