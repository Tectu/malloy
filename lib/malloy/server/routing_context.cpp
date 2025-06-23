#include "routing_context.hpp"
#include "listener.hpp"
#include "routing/router.hpp"
#if MALLOY_FEATURE_TLS
    #include "../core/tls/manager.hpp"
#endif

#include <spdlog/logger.h>
#include <spdlog/sinks/null_sink.h>

#include <memory>

using namespace malloy::server;

routing_context::routing_context(config cfg) :
    m_cfg{std::move(cfg)},
    m_router{new server::router{m_cfg.logger != nullptr ? m_cfg.logger->clone("router") : nullptr,
                                m_cfg.agent_string}}
{
    m_cfg.validate();

    // If no connection logger is provided, use the null logger.
    if (!m_cfg.connection_logger)
        m_cfg.connection_logger = spdlog::null_logger_mt("connection");
}

#if MALLOY_FEATURE_TLS
    bool
    routing_context::init_tls(
        const std::filesystem::path& cert_path,
        const std::filesystem::path& key_path
    )
    {
        // Sanity check cert_path
        if (!std::filesystem::is_regular_file(cert_path)) {
            m_cfg.logger->critical("could not create TLS context: invalid certificate file path: {}", cert_path.string());
            return false;
        }

        // Sanity check key_path
        if (!std::filesystem::is_regular_file(key_path)) {
            m_cfg.logger->critical("could not create TLS context: invalid key file path: {}", key_path.string());
            return false;
        }

        // Create the context
        m_tls_ctx = tls::manager::make_context(cert_path, key_path);

        return m_tls_ctx != nullptr;
    }

    bool
    routing_context::init_tls(const std::string& cert, const std::string& key)
    {
        m_tls_ctx = tls::manager::make_context(cert, key);

        return m_tls_ctx != nullptr;
    }
#endif
