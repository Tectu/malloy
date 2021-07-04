#include "controller.hpp"

#if MALLOY_FEATURE_TLS 
    #include <boost/certify/extensions.hpp>
    #include <boost/certify/https_verification.hpp>
#endif

using namespace malloy::client;

namespace fs = std::filesystem;
auto controller::run() -> bool
{
    if (!start()) {
        return false;
    }
    io_ctx().run();
    return true;
}

#if MALLOY_FEATURE_TLS
    bool controller::init_tls()
    {
        m_tls_ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12_client);
        m_tls_ctx->set_verify_mode(boost::asio::ssl::verify_peer | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
        m_tls_ctx->set_default_verify_paths();
        boost::certify::enable_native_https_server_verification(*m_tls_ctx);

        return true;
    }

    void controller::load_ca_file(const std::filesystem::path& file) {
        if (!fs::exists(file)) {
            throw std::invalid_argument{fmt::format("add_tls_keychain passed '{}', which does not exist", file.string())};
        }
        check_tls();
        
        m_tls_ctx->load_verify_file(file.string());
    }

    void controller::load_cert(const std::string& contents) {
        check_tls();
        m_tls_ctx->use_certificate_chain(malloy::buffer(contents));
    }
#endif // MALLOY_FEATURE_TLS

