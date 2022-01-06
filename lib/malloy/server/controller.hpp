#pragma once

#include "malloy/core/controller.hpp"
#include "malloy/server/listener.hpp"
#include "malloy/server/routing/router.hpp"


#include <memory>
#include <filesystem>
#include <string>

namespace boost::asio::ssl
{
    class context;
}

namespace malloy::server
{

    class router;
    class listener;

    /**
     * This is a high-level controller for convenience.
     * It will setup the necessary I/O context and worker threads.
     *
     * @brief A high-level controller.
     */
    class controller
    {
    public:
        /**
         * Controller configuration.
         */
        struct config :
            malloy::controller::config
        {
            /**
             * The interface to bind to.
             */
            std::string interface = "127.0.0.1";

            /**
             * The port to listen on.
             */
            std::uint16_t port = 8080;

            /**
             * The root path for HTTP documents.
             *
             * This is a filesystem path that can either be an absolute or relative
             * to the working directory.
             */
            std::filesystem::path doc_root = ".";

            /**
             * @brief Agent string used for connections
             * @details Set as the Server field in http headers
             */
            std::string agent_string{"malloy-server"};
        };

        controller(config cfg);
        controller(const controller& other) = delete;
        controller(controller&& other) noexcept = default;
        ~controller() = default;

        controller& operator=(const controller& rhs) = delete;
        controller& operator=(controller&& rhs) noexcept = default;

        #if MALLOY_FEATURE_TLS
            /**
             * Initialize the TLS context.
             *
             * @note This must be called after `init()` but before `start()` if TLS is to be used.
             *
             * @param cert_path Path to the certificate file.
             * @param key_path Path to the key file.
             * @return Whether the initialization was successful.
             */
            bool init_tls(const std::filesystem::path& cert_path, const std::filesystem::path& key_path);

            bool init_tls(const std::string& cert, const std::string& key);
        #endif

        /**
         * Get the top-level HTTP router.
         *
         * @return The top-level HTTP router.
         */
        [[nodiscard]]
        constexpr const malloy::server::router& router() const noexcept
        {
            return m_router;
        }
        [[nodiscard]]
        constexpr auto router() noexcept -> class router& {
            return m_router;
        }

    private:
        using run_result_t = malloy::detail::controller_run_result<std::shared_ptr<malloy::server::listener>>;
        [[nodiscard("ignoring result will cause the server to instantly stop")]] friend auto start(controller&& ctrl) -> run_result_t
        {
            // Log
            ctrl.m_cfg.logger->debug("starting server.");
            auto ioc = std::make_unique<boost::asio::io_context>();

            // Create the listener
            auto l = std::make_shared<malloy::server::listener>(
                ctrl.m_cfg.logger->clone("listener"),
                *ioc,
#if MALLOY_FEATURE_TLS
                std::move(ctrl.m_tls_ctx),
#else
                nullptr,
#endif
                boost::asio::ip::tcp::endpoint{boost::asio::ip::make_address(ctrl.m_cfg.interface), ctrl.m_cfg.port},
                std::make_shared<class router>(std::move(ctrl.m_router)),
                std::make_shared<std::filesystem::path>(ctrl.m_cfg.doc_root),
                ctrl.m_cfg.agent_string);

            // Run the listener
            l->run();
            return run_result_t{ctrl.m_cfg, std::move(l), std::move(ioc)};
        }
        config m_cfg;
#if MALLOY_FEATURE_TLS
        std::unique_ptr<boost::asio::ssl::context> m_tls_ctx;
#endif
        malloy::server::router m_router;
    };

}
