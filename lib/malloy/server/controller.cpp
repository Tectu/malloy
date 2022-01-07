#include "controller.hpp"
#include "listener.hpp"
#include "routing/router.hpp"
#if MALLOY_FEATURE_TLS
    #include "../core/tls/manager.hpp"
#endif

#include <spdlog/logger.h>

#include <memory>

using namespace malloy::server;

controller::controller(config cfg) :
    m_cfg{std::move(cfg)},
    m_router{m_cfg.logger != nullptr ? m_cfg.logger->clone("router") : nullptr, m_cfg.agent_string}
{
    m_cfg.validate();
}


#if MALLOY_FEATURE_TLS
    bool controller::init_tls(
        const std::filesystem::path& cert_path,
        const std::filesystem::path& key_path
    )
    {
        // Sanity check cert
        if (!std::filesystem::is_regular_file(cert_path)) {
            m_cfg.logger->critical("could not create TLS context: invalid certificate file path: {}", cert_path.string());
            return false;
        }

        // Sanity check key_path
        if (!std::filesystem::is_regular_file(key_path)) {
            m_cfg.logger->critical("could not create TLS context: invalid key file path: {}", key_path.string());
        }

        // Create the context
        m_tls_ctx = tls::manager::make_context(cert_path, key_path);

        return true;
    }

    bool controller::init_tls(const std::string& cert, const std::string& key)
    {
        m_tls_ctx = tls::manager::make_context(cert, key);
        return m_tls_ctx != nullptr;
    }
#endif
