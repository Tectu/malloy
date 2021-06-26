#pragma once

#include "../controller.hpp"
#include "../http/request.hpp"
#include "../http/response.hpp"
#include "../websocket/types.hpp"
#include "malloy/error.hpp"
#include "malloy/client/websocket/connection.hpp"

#include <boost/asio/strand.hpp>

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
        http_request(http::request<> req);

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
        [[nodiscard]]
        auto
            make_websocket_connection(const std::string& host, std::uint16_t port, const std::string& resource,
                std::invocable<malloy::error_code, std::shared_ptr<websocket::connection>> auto&& handler)
        {
            // Create connection
            auto resolver = std::make_shared<boost::asio::ip::tcp::resolver>(boost::asio::make_strand(io_ctx()));
            resolver->async_resolve(
                host,
                port,
                [this, resolver, done = std::forward<decltype(handler)>(handler)](auto ec, auto results){

                if (ec) {
                    std::invoke(std::forward<decltype(done)>(done), ec, nullptr);
                }
                else {
                    auto conn = std::make_shared<websocket::connection>(m_cfg.logger->clone("connection"), malloy::websocket::stream{
                        boost::beast::tcp_stream{boost::asio::make_strand(io_ctx())}
                        });
                    conn->connect(results, resource, [conn, done = std::forward<decltype(handler)>(handler)](auto ec) {
                        if (ec) {
                            std::invoke(std::forward<decltype(handler)>(done), ec, nullptr);
                        }
                        else {
                            std::invoke(std::forward<decltype(handler)>(done), ec, conn);
                        }
                    });
                }
            }
            );
        }

    private:
        std::shared_ptr<boost::asio::ssl::context> m_tls_ctx;
    };

}
