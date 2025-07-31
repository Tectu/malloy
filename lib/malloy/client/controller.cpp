#include "controller.hpp"

#if MALLOY_FEATURE_TLS 
    #include <boost/certify/extensions.hpp>
    #include <boost/certify/https_verification.hpp>
#endif
#include <fmt/format.h>

using namespace malloy::client;

controller::controller(config cfg) :
    m_cfg{std::move(cfg)}
{
    m_cfg.validate();
}

#if MALLOY_FEATURE_TLS
    bool
    controller::init_tls()
    {
        m_tls_ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12_client);
        m_tls_ctx->set_verify_mode(boost::asio::ssl::verify_peer | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
        m_tls_ctx->set_default_verify_paths();
        boost::certify::enable_native_https_server_verification(*m_tls_ctx);

        return true;
    }

    std::expected<void, std::string>
    controller::add_ca_file(const std::filesystem::path& file)
    {
        if (!std::filesystem::exists(file))
            return std::unexpected(fmt::format("add_tls_keychain passed '{}', which does not exist", file.string()));

        if (!tls_context_valid())
            return std::unexpected("TLS context not initialized");

        m_tls_ctx->load_verify_file(file.string());

        return { };
    }

    std::expected<void, std::string>
    controller::add_ca(const std::string& contents)
    {
        if (!tls_context_valid())
            return std::unexpected("TLS context not initialized");

        m_tls_ctx->add_certificate_authority(malloy::buffer(contents));

        return { };
    }
#endif // MALLOY_FEATURE_TLS
