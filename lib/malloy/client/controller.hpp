#pragma once

#include "../controller.hpp"
#include "../http/request.hpp"
#include "../http/response.hpp"
#include "malloy/http/type_traits.hpp"
#include "../websocket/types.hpp"
#include "malloy/error.hpp"
#include "malloy/client/type_traits.hpp"
#include "malloy/client/websocket/connection.hpp"
#include "malloy/client/http/connection_plain.hpp"

#if MALLOY_FEATURE_TLS
    #include "malloy/client/http/connection_tls.hpp"

    #include <boost/beast/ssl.hpp>
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
            using header_type = boost::beast::http::response_header<>;
            using value_type = std::string;

            auto body_for(const header_type&) const -> std::variant<boost::beast::http::string_body> {
                return {};
            }
            void setup_body(const header_type&, std::string&) const {}
        };
        static_assert(malloy::client::concepts::response_filter<default_resp_filter>, "default_resp_filter must satisfy response_filter");
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
         
        template<malloy::http::concepts::body ReqBody, typename Callback, concepts::response_filter Filter = detail::default_resp_filter>
        [[nodiscard]]
        auto http_request(malloy::http::request<ReqBody> req, Callback&& done, Filter filter = {}) -> std::future<malloy::error_code> {

            // Create connection
            auto conn = std::make_shared<http::connection_plain<ReqBody, Filter, std::decay_t<Callback>>>(
                m_cfg.logger->clone(m_cfg.logger->name() + " | HTTP connection"),
                io_ctx()
                );
            std::promise<malloy::error_code> prom;
            auto err_channel = prom.get_future();
            conn->run(
                std::to_string(req.port()).c_str(),
                req,
                std::move(prom),
                std::move(done),
                std::move(filter)
            );
            return err_channel;


        }

        #if MALLOY_FEATURE_TLS
            /**
             * Perform a TLS encrypted HTTPS request.
             *
             * @param req The HTTPS request.
             *
             * @return The corresponding response.
             */

        template<malloy::http::concepts::body ReqBody, typename Callback, concepts::response_filter Filter = detail::default_resp_filter>
        [[nodiscard]]
        auto https_request(malloy::http::request<ReqBody> req, Callback&& done, Filter filter = {}) -> std::future<malloy::error_code> {
            // Check whether TLS context was initialized
            if (!m_tls_ctx)
                throw std::logic_error("TLS context not initialized.");


            auto conn = std::make_shared<http::connection_tls<ReqBody, Filter, std::decay_t<Callback>>>(
                m_cfg.logger->clone(m_cfg.logger->name() + " | HTTP connection"),
                io_ctx(),
                *m_tls_ctx
                );
            std::promise<malloy::error_code> prom;
            auto err_channel = prom.get_future();
            conn->run(
                std::to_string(req.port()).c_str(),
                req,
                std::move(prom),
                std::move(done),
                std::move(filter));
            return err_channel;
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
        void make_websocket_connection(const std::string& host, std::uint16_t port, const std::string& resource,
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
