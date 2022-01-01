#pragma once

#include "http/connection_plain.hpp"
#include "type_traits.hpp"
#include "websocket/connection.hpp"
#include "../core/controller.hpp"
#include "../core/http/request.hpp"
#include "../core/http/response.hpp"
#include "../core/http/type_traits.hpp"
#include "../core/error.hpp"
#include "malloy/core/http/utils.hpp"


#if MALLOY_FEATURE_TLS
    #include "http/connection_tls.hpp"

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

        template<typename T>
        concept ws_accept_completion_tkn = boost::asio::completion_token_for<T, void(malloy::error_code, std::shared_ptr<websocket::connection>)>;

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

            /**
             * @brief Agent string used for connections
             * @details Set as the User-Agent in http headers
             */
            std::string user_agent{"malloy-client"};
        };

        controller() = default;
        ~controller() override = default;

        [[nodiscard("init may fail")]]
        auto init(config cfg) -> bool;

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
        requires concepts::http_completion_token<Callback, tmp::bodies_for_t<std::remove_cvref_t<Filter>>>
        auto http_request(malloy::http::request<ReqBody> req, Callback&& done, Filter filter = {})
        {
            return make_http_connection<false>(std::move(req), std::forward<Callback>(done), std::move(filter));
        }

#if MALLOY_FEATURE_TLS
        /**
         * Same as http_request() but encrypted with TLS.
         *
         * @sa http_request()
         */
        template<malloy::http::concepts::body ReqBody, typename Callback, concepts::response_filter Filter = detail::default_resp_filter>
        requires concepts::http_callback<Callback, Filter>
        auto https_request(malloy::http::request<ReqBody> req, Callback&& done, Filter filter = {})
        {
            return make_http_connection<true>(std::move(req), std::forward<Callback>(done), std::move(filter));
        }

        /**
         * @brief Same as ws_connect() but uses TLS
         * @warning tls_init MUST be called before this
         *
         * @sa ws_connect()
         */
        auto wss_connect(
            const std::string& host,
            std::uint16_t port,
            const std::string& resource,
            detail::ws_accept_completion_tkn auto&& handler)
        {
            check_tls();

            // Create connection
            return make_ws_connection<true>(host, port, resource, std::forward<decltype(handler)>(handler));
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
        auto ws_connect(
            const std::string& host,
            std::uint16_t port,
            const std::string& resource,
            detail::ws_accept_completion_tkn auto&& handler)
        {
            // Create connection
            return make_ws_connection<false>(host, port, resource, std::forward<decltype(handler)>(handler));
        }

        /**
         * @brief Block until all queued async actions completed
         * @return whether starting was successful
         */
        auto run() -> bool;

        auto start() -> bool;

    protected:

    private:
        std::shared_ptr<boost::asio::ssl::context> m_tls_ctx;
        config m_cfg;

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

        template<bool isHttps, malloy::http::concepts::body Body, typename Callback, typename Filter>
        auto make_http_connection(malloy::http::request<Body>&& req, Callback&& cb, Filter&& filter)
        {
            auto make_conn = [this](auto&& act, auto&&... args) {
#if MALLOY_FEATURE_TLS
                if constexpr (isHttps) {
                    if (!init_tls()) {
                        throw std::runtime_error{"failed to init tls"};
                    }
                    std::make_shared<http::connection_tls<Body, Filter, std::remove_cvref_t<decltype(act)>>>(
                        m_cfg.logger->clone(m_cfg.logger->name() + " | HTTP connection"),
                        io_ctx(),
                        *m_tls_ctx, std::forward<decltype(act)>(act), std::forward<decltype(args)>(args)...);
                    return;
                }
#endif
                std::make_shared<http::connection_plain<Body, Filter, std::remove_cvref_t<decltype(act)>>>(
                    m_cfg.logger->clone(m_cfg.logger->name() + " | HTTP connection"),
                    io_ctx(), std::forward<decltype(act)>(act), std::forward<decltype(args)>(args)...);
            };
            if (!malloy::http::has_field(req, malloy::http::field::user_agent)) {
                    req.set(malloy::http::field::user_agent, m_cfg.user_agent);
            }
            auto do_init = [filter = std::forward<Filter>(filter), &req, &make_conn](auto&& act) mutable {
                make_conn(std::forward<decltype(act)>(act), std::to_string(req.port()), req, std::forward<Filter>(filter));
            };
            return boost::asio::async_initiate<Callback, void(error_code, tmp::filter_resp_t<std::remove_cvref_t<Filter>>)>(std::move(do_init), cb);

        }

        template<bool isSecure>
        auto make_ws_connection(
            const std::string& host,
            std::uint16_t port,
            const std::string& resource,
            detail::ws_accept_completion_tkn auto&& handler
        )
        {
            // Create connection
            auto resolver = std::make_shared<boost::asio::ip::tcp::resolver>(boost::asio::make_strand(io_ctx()));
            auto on_resolve = [this, resolver, resource, handler = std::forward<decltype(handler)>(handler)](auto act, error_code ec, auto results) mutable {
                (void)handler;
                    if (ec) {
                        std::invoke(act, ec, std::shared_ptr<websocket::connection>(nullptr));
                    } else {
                        auto conn = websocket::connection::make(m_cfg.logger->clone("connection"), [this]() -> malloy::websocket::stream {
#if MALLOY_FEATURE_TLS
                            if constexpr (isSecure) {
                                return malloy::websocket::stream{boost::beast::ssl_stream<boost::beast::tcp_stream>{
                                    boost::beast::tcp_stream{boost::asio::make_strand(io_ctx())}, *m_tls_ctx}};
                            } else
#endif
                                return malloy::websocket::stream{boost::beast::tcp_stream{boost::asio::make_strand(io_ctx())}};
                        }(), m_cfg.user_agent);

                        conn->connect(results, resource, [conn, act = std::move(act)](auto ec) mutable {
                            if (ec) {
                            std::invoke(act, ec, std::shared_ptr<websocket::connection>{nullptr});
                            } else {
                            std::invoke(act, ec, conn);
                            }
                        });
                    } };

            auto resolve_wrapper = [host, port, resolver, on_resolve = std::move(on_resolve)](auto done) mutable {
                resolver->async_resolve(
                    host,
                    std::to_string(port),
                    [done = std::forward<decltype(done)>(done), on_resolve = std::move(on_resolve)](auto ec, auto rs) mutable { on_resolve(std::forward<decltype(done)>(done), ec, rs); });
            };
            return boost::asio::async_initiate<decltype(handler), void(error_code, std::shared_ptr<websocket::connection>)>(std::move(resolve_wrapper), handler);
        }
    };

}    // namespace malloy::client
