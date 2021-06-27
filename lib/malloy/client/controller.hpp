#pragma once

#include "../controller.hpp"
#include "../http/request.hpp"
#include "../http/response.hpp"
#include "malloy/http/type_traits.hpp"
#include "../websocket/types.hpp"
#include "malloy/error.hpp"
#include "malloy/client/type_traits.hpp"

#if MALLOY_FEATURE_TLS
    #include "http/connection_tls.hpp"

    #include <boost/beast/ssl.hpp>
    #include <boost/certify/extensions.hpp>
    #include <boost/certify/https_verification.hpp>
#endif



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

    namespace detail {
        struct default_resp_filter {
            using response_type = malloy::http::response<>;
            void setup_body(const typename response_type::header_type&, typename response_type::body_type::value_type&) const {}
        };
        static_assert(concepts::resp_filter<default_resp_filter>, "default_resp_filter must satisfy resp_filter");
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
         
        template<malloy::http::concepts::body ReqBody, typename Callback, malloy::concepts::resp_filter Filter = detail::default_resp_filter>
        void http_request(malloy::http::request<ReqBody> req, Callback&& done, Filter filter = {}) {

            // Create connection
            auto conn = std::make_shared<http::connection_plain<ReqBody, Filter>>(
                m_cfg.logger->clone(m_cfg.logger->name() + " | HTTP connection"),
                io_ctx()
                );
            conn->run(
                std::to_string(req.port()).c_str(),
                req,
                std::forward<Callback>(done),
                std::move(filter)
            );


        }

        #if MALLOY_FEATURE_TLS
            /**
             * Perform a TLS encrypted HTTPS request.
             *
             * @param req The HTTPS request.
             *
             * @return The corresponding response.
             */

        template<malloy::http::concepts::body ReqBody, typename Callback, malloy::concepts::resp_filter Filter = detail::default_resp_filter>
        void https_request(malloy::http::request<ReqBody> req, Callback&& done, Filter filter = {}) {
            // Check whether TLS context was initialized
            if (!m_tls_ctx)
                throw std::logic_error("TLS context not initialized.");


            auto conn = std::make_shared<http::connection_plain<ReqBody, Filter>>(
                m_cfg.logger->clone(m_cfg.logger->name() + " | HTTP connection"),
                io_ctx(),
                *m_tls_ctx
                );
            conn->run(
                std::to_string(req.port()),
                req,
                std::forward<Callback>(done),
                std::move(filter));
        }
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
                std::to_string(port),
                [this, resolver, done = std::forward<decltype(handler)>(handler), resource](auto ec, auto results) mutable {

                if (ec) {
                    std::invoke(std::forward<decltype(done)>(done), ec, std::shared_ptr<websocket::connection>{nullptr});
                }
                else {
                    auto conn = websocket::connection::make(m_cfg.logger->clone("connection"), malloy::websocket::stream{
                        boost::beast::websocket::stream<boost::beast::tcp_stream>{boost::beast::tcp_stream{boost::asio::make_strand(io_ctx())}}
                        });
                    conn->connect(results, resource, [conn, done = std::forward<decltype(done)>(done)](auto ec) mutable {
                        if (ec) {
                            std::invoke(std::forward<decltype(handler)>(done), ec, std::shared_ptr<websocket::connection>{nullptr});
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
