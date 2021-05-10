#pragma once

#include "websocket/types.hpp"

#include <boost/asio/io_context.hpp>

#include <memory>
#include <filesystem>
#include <future>
#include <string>
#include <thread>
#include <vector>

namespace boost::asio::ssl
{
    class context;
}

namespace spdlog
{
    class logger;
}

namespace malloy::http::server
{
    class router;
}

namespace malloy::server
{

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
         * Controller configuration
         */
        struct config
        {
            /**
             * The interface to bind to.
             */
            std::string interface          = "127.0.0.1";

            /**
             * The port to listen on.
             */
            std::uint16_t port             = 8080;

            /**
             * The root path for HTTP documents.
             *
             * This is a filesystem path that can either be an absolute or relative
             * to the working directory.
             */
            std::filesystem::path doc_root = ".";

            /**
             * The number of worked threads for the I/O context to use.
             */
            std::size_t num_threads        = 1;

            /**
             * The logger instance to use.
             * A logger will be automatically created if none was provided.
             */
            std::shared_ptr<spdlog::logger> logger;

            config() = default;
            config(const config& other) = default;
            config(config&& other) noexcept = default;
            virtual ~config() = default;

            config& operator=(const config& cfg) = default;
            config& operator=(config&& cfg) noexcept = default;
        };

        controller() = default;
        controller(const controller& other) = delete;
        controller(controller&& other) noexcept = delete;
        virtual ~controller();

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
        bool init(config cfg);

        #if MALLOY_FEATURE_TLS
            /**
             * Initialize the TLS context.
             *
             * @note This must be called after `init()` but before `start()` if TLS is to be used.
             *
             * @return Whether the initialization was successful.
             */
            bool init_tls();
        #endif

        /**
         * Start the server. This function will not return until the server is stopped.
         *
         * @return Whether starting the server was successful.
         */
        bool start();

        /**
         * Stop the server.
         *
         * @return A future indicating when the stopping operation was completed.
         */
        std::future<void> stop();

        /**
         * Get the top-level HTTP router.
         *
         * @return The top-level HTTP router.
         */
        [[nodiscard]]
        std::shared_ptr<malloy::http::server::router> router() const noexcept
        {
            return m_router;
        }

        /**
         * This function can be used to register a handler for incoming websocket
         * requests.
         *
         * @brief Set the websocket handler.
         *
         * @param handler The handler to use.
         */
        void set_websocket_handler(malloy::websocket::handler_type handler);

    private:
        bool m_init_done = false;
        config m_cfg;
        std::shared_ptr<listener> m_listener;
        std::vector<std::thread> m_threads;
        boost::asio::io_context m_io_ctx;
        std::shared_ptr<boost::asio::ssl::context> m_tls_ctx;
        std::shared_ptr<malloy::http::server::router> m_router;
    };

}
