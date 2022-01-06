#pragma once

#include "malloy/core/controller.hpp"
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
    class controller :
        public malloy::controller
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

        controller() = default;
        controller(const controller& other) = delete;
        controller(controller&& other) noexcept = delete;
        ~controller() = default;

        controller& operator=(const controller& rhs) = delete;
        controller& operator=(controller&& rhs) noexcept = delete;

        /**
         * Initialize the controller.
         *
         * @note This function must be called before any other.
         *
         * @param cfg The configuration to use.
         * @return Whether the initialization was successful.
         */
         [[nodiscard("init may fail")]]
        bool init(config cfg);

        /**
         * Start the server. This function will not return until the server is stopped.
         *
         * @note The controller must not be reused after this call
         *
         * @return Whether starting the server was successful.
         */
        bool start() &&;

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
        config m_cfg;
        std::shared_ptr<listener> m_listener;
#if MALLOY_FEATURE_TLS
        std::unique_ptr<boost::asio::ssl::context> m_tls_ctx;
#endif
        malloy::server::router m_router;
    };

}
