#include "controller.hpp"
#include "listener.hpp"
#include "routing/router.hpp"
#if MALLOY_FEATURE_TLS
    #include "../core/tls/manager.hpp"
#endif

#include <spdlog/logger.h>

#include <memory>

using namespace malloy::server;

auto controller::init(config cfg) -> bool {
    if (!malloy::controller::init(cfg)) {
        return false;
    }
    // Grab the config
    m_cfg = std::move(cfg);

    // Create the top-level router
    m_router = std::make_shared<malloy::server::router>(m_cfg.logger->clone("router"), m_cfg.agent_string);
    return true;
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

bool controller::start()
{
    // Log
    m_cfg.logger->debug("starting server.");

    // Create the listener
    m_listener = std::make_shared<malloy::server::listener>(
        m_cfg.logger->clone("listener"),
        io_ctx(),
        m_tls_ctx,
        boost::asio::ip::tcp::endpoint{ boost::asio::ip::make_address(m_cfg.interface), m_cfg.port },
        m_router,
        std::make_shared<std::filesystem::path>(m_cfg.doc_root),
        m_cfg.agent_string);

    // Run the listener
    m_listener->run();

    // Base class
    if (!root_start(m_cfg))
        return false;

    return true;
}
