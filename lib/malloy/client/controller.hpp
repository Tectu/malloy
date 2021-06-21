#pragma once

#include "../controller.hpp"
#include "../http/request.hpp"
#include "../http/response.hpp"
#include "../websocket/types.hpp"

#include <spdlog/logger.h>

namespace boost::asio::ssl
{
    class context;
}

namespace malloy::client
{
    namespace websocket
    {
        class connection_plain;
    }

    /**
     * High-level controller for client activities.
     */
    class controller :
        public malloy::controller
    {
    public:
        struct config :
            malloy::controller::config
        {
        };

        controller() = default;
        ~controller() = default;

        #if MALLOY_FEATURE_TLS
            [[nodiscard("init might fail")]]
            bool init_tls();
        #endif

        /**
         * Perform a plain (unencrypted) HTTP request.
         *
         * @param req The HTTP request.
         *
         * @return The corresponding response.
         */
        [[nodiscard]]
        std::future<http::response<>>
        http_request(http::request req);

        #if MALLOY_FEATURE_TLS
            /**
             * Perform a TLS encrypted HTTPS request.
             *
             * @param req The HTTPS request.
             *
             * @return The corresponding response.
             */
            [[nodiscard]]
            std::future<http::response<>>
            https_request(http::request req);
        #endif

        /**
         * Create a websocket connection.
         *
         * @tparam Connection The type of connection to use.
         * @param host The host.
         * @param port The port.
         * @param endpoint The endpoint.
         * @param handler Handler that gets called when data is received.
         *
         * @return The connection.
         */
        template<class Connection>
        [[nodiscard]]
        std::shared_ptr<Connection>
        make_websocket_connection(const std::string& host, std::uint16_t port, const std::string& endpoint, malloy::websocket::handler_t&& handler)
        {
            // Sanity check
            if (!handler)
                return { };

            // Create connection
            auto conn = std::make_shared<Connection>(m_cfg.logger->clone("connection"), io_ctx(), std::move(handler));

            // Launch the connection
            conn->connect(host, std::to_string(port), endpoint);

            return conn;
        }

    private:
        std::shared_ptr<boost::asio::ssl::context> m_tls_ctx;
    };

}
