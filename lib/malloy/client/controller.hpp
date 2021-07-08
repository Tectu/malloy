#pragma once

#include "../controller.hpp"
#include "../http/request.hpp"
#include "../http/response.hpp"
#include "malloy/http/type_traits.hpp"
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

#include <filesystem>

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

    namespace detail
    {

        /** 
         * @brief Default filter provided to ease use of interface
         * @details Provides an implementation of response_filter @ref client_concepts for 
         * use as the default value of Filter types so that users to not have to
         * deal with them unnecessarily
         */
        struct default_resp_filter
        {
            using response_type = malloy::http::response<>;
            using header_type = boost::beast::http::response_header<>;
            using value_type = std::string;

            auto body_for(const header_type&) const -> std::variant<boost::beast::http::string_body>
            {
                return {};
            }
            void setup_body(const header_type&, std::string&) const {}
        };

        static_assert(malloy::client::concepts::response_filter<default_resp_filter>, "default_resp_filter must satisfy response_filter");
    }    // namespace detail

    /**
     * High-level controller for client activities.
     */
    class controller :
        public malloy::controller
    {
    public:
        struct config :
            malloy::controller::config {
        };

        controller() = default;
        ~controller() = default;

#if MALLOY_FEATURE_TLS
        /**
         * Initialize the TLS context.
         *
         * @return Whether initialization was successful.
         */
        [[nodiscard("init might fail")]] bool init_tls();
#endif

        /**
         * Perform a plain (unencrypted) HTTP request.
         *
         * @param req The HTTP request.
         * @param done Callback invoked on completion. Must satisfy http_callback (@ref client_concepts) with Filter
         * @param filter Filter to use when parsing the response. Must satisfy
         * response_filter @ref client_concepts
         *
         * @return A future for reporting errors. Will be filled with a falsy
         * error_code on success.
         *
         * @sa https_request()
         */
        template<malloy::http::concepts::body ReqBody, typename Callback, concepts::response_filter Filter = detail::default_resp_filter>
        requires concepts::http_callback<Callback, Filter>
        [[nodiscard]] auto http_request(malloy::http::request<ReqBody> req, Callback&& done, Filter filter = {}) -> std::future<malloy::error_code>
        {

            // Create connection
            auto conn = std::make_shared<http::connection_plain<ReqBody, Filter, std::decay_t<Callback>>>(
                m_cfg.logger->clone(m_cfg.logger->name() + " | HTTP connection"),
                io_ctx()
            );

            // Run
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
         * Same as http_request() but encrypted with TLS.
         *
         * @sa http_request()
         */
        template<malloy::http::concepts::body ReqBody, typename Callback, concepts::response_filter Filter = detail::default_resp_filter>
        requires concepts::http_callback<Callback, Filter>
        [[nodiscard]] auto https_request(malloy::http::request<ReqBody> req, Callback&& done, Filter filter = {}) -> std::future<malloy::error_code>
        {
            check_tls();

            // Create connection
            auto conn = std::make_shared<http::connection_tls<ReqBody, Filter, std::decay_t<Callback>>>(
                m_cfg.logger->clone(m_cfg.logger->name() + " | HTTP connection"),
                io_ctx(),
                *m_tls_ctx
            );

            // Run
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

        /**
         * @brief Same as ws_connect() but uses TLS
         * @warning tls_init MUST be called before this
         *
         * @sa ws_connect()
         */
        void wss_connect(
            const std::string& host,
            std::uint16_t port,
            const std::string& resource,
            std::invocable<malloy::error_code, std::shared_ptr<websocket::connection>> auto&& handler)
        {
            check_tls();

            // Create connection
            make_ws_connection<true>(host, port, resource, std::forward<decltype(handler)>(handler));
        }

        /**
         * @brief Load a certificate authority for use with TLS validation
         * @warning tls_init() MUST be called before this.
         * @param file The path to the certificate to be added to the keychain
         *
         * @sa add_ca()
         */
        void add_ca_file(const std::filesystem::path& file);
        /**
         * @brief Like add_ca_file(std::filesystem::path) but loads from an in-memory string
         * @warning tls_init() MUST be called before this.
         * @param contents The certificate to be added to the keychain
         *
         * @sa add_ca_file()
         */
        void add_ca(const std::string& contents);
#endif

        /**
         * Create a websocket connection.
         *
         * @param host The host.
         * @param port The port.
         * @param resource The suburl to connect to (e.g. `/api/websocket`)
         * @param handler Callback invoked when the connection is established
         * (successfully or otherwise). Must satisfy `f(e, c)`, where:
         * - `f` is the callback
         * - `e` is `malloy::error_code` 
         * - `c` is `std::shared_ptr<websocket::connection>`
         * @warning If the error code passed to `handler` is truthy (an error) the
         * connection will be `nullptr`
         *
         * @sa wss_connect()
         */
        void ws_connect(
            const std::string& host,
            std::uint16_t port,
            const std::string& resource,
            std::invocable<malloy::error_code, std::shared_ptr<websocket::connection>> auto&& handler)
        {
            // Create connection
            make_ws_connection<false>(host, port, resource, std::forward<decltype(handler)>(handler));
        }

        /**
         * @brief Block until all queued async actions completed
         * @return whether starting was successful
         */
        auto run() -> bool;

    private:
        std::shared_ptr<boost::asio::ssl::context> m_tls_ctx;

        /**
         * Checks whether the TLS context was initialized.
         * @note This will throw if the TLS context was not initialized.
         */
        void check_tls() const
        {
            // Check whether TLS context was initialized
            if (!m_tls_ctx)
                throw std::logic_error("TLS context not initialized.");
        }

        template<bool isSecure>
        void make_ws_connection(
            const std::string& host,
            std::uint16_t port,
            const std::string& resource,
            std::invocable<malloy::error_code, std::shared_ptr<websocket::connection>> auto&& handler
        )
        {
            // Create connection
            auto resolver = std::make_shared<boost::asio::ip::tcp::resolver>(boost::asio::make_strand(io_ctx()));
            resolver->async_resolve(
                host,
                std::to_string(port),
                [this, resolver, done = std::forward<decltype(handler)>(handler), resource](auto ec, auto results) mutable {
                    if (ec) {
                        std::invoke(std::forward<decltype(done)>(done), ec, std::shared_ptr<websocket::connection>{nullptr});
                    } else {
                        auto conn = websocket::connection::make(m_cfg.logger->clone("connection"), [this]() -> malloy::websocket::stream {
#if MALLOY_FEATURE_TLS
                            if constexpr (isSecure) {
                                return malloy::websocket::stream{boost::beast::ssl_stream<boost::beast::tcp_stream>{
                                    boost::beast::tcp_stream{boost::asio::make_strand(io_ctx())}, *m_tls_ctx}};
                            } else
#endif
                                return malloy::websocket::stream{boost::beast::tcp_stream{boost::asio::make_strand(io_ctx())}};
                        }());

                        conn->connect(results, resource, [conn, done = std::forward<decltype(done)>(done)](auto ec) mutable {
                            if (ec) {
                                std::invoke(std::forward<decltype(handler)>(done), ec, std::shared_ptr<websocket::connection>{nullptr});
                            } else {
                                std::invoke(std::forward<decltype(handler)>(done), ec, conn);
                            }
                        });
                    }
                }
            );
        }
    };

}    // namespace malloy::client
