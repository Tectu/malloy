#pragma once

#include "type_traits.hpp"
#include "http/connection_plain.hpp"
#include "websocket/connection.hpp"
#include "../core/controller.hpp"
#include "../core/error.hpp"
#include "../core/detail/controller_run_result.hpp"
#include "../core/http/request.hpp"
#include "../core/http/response.hpp"
#include "../core/http/type_traits.hpp"
#include "../core/http/url.hpp"
#include "../core/http/utils.hpp"
#include "../core/tcp/stream.hpp"
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

            [[nodiscard]]
            std::variant<boost::beast::http::string_body>
            body_for(const header_type&) const
            {
                return {};
            }

            void
            setup_body(const header_type&, std::string&) const
            {
            }
        };

        static_assert(malloy::client::concepts::response_filter<default_resp_filter>, "default_resp_filter must satisfy response_filter");

    } // namespace detail

    /**
     * High-level controller for client activities.
     */
    class controller
    {
    public:
        /**
         * Session type.
         */
        using session = malloy::detail::controller_run_result<std::shared_ptr<boost::asio::ssl::context>>;

        /**
         * Configuration type.
         */
        struct config :
            malloy::controller::config
        {
            /**
             * @brief Agent string used for connections
             * @details Set as the User-Agent in http headers
             */
            std::string user_agent{"malloy"};

            /**
             * The maximum allowed response body size in bytes.
             */
            std::uint64_t body_limit = 100'000'000;
        };

        /**
         * Constructor.
         *
         * @param cfg The configuration.
         */
        explicit
        controller(config cfg);

        /**
         * Destructor.
         */
        ~controller() = default;

#if MALLOY_FEATURE_TLS
        /**
         * Initialize the TLS context.
         *
         * @return Whether initialization was successful.
         */
        [[nodiscard("init might fail")]]
        bool
        init_tls();
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
        template<
            malloy::http::concepts::body ReqBody,
            typename Callback, concepts::response_filter Filter = detail::default_resp_filter
        >
        requires concepts::http_callback<Callback, Filter>
        [[nodiscard]]
        std::future<malloy::error_code>
        http_request(
            malloy::http::request<ReqBody> req,
            Callback&& done,
            Filter filter = {}
        )
        {
            return make_http_connection<false>(std::move(req), std::forward<Callback>(done), std::move(filter));
        }

        /**
         * Perform an HTTP request.
         *
         * @detail This will either issue a plain (unencrypted) or TLS request based on the scheme in the URL (http:// vs. https://).
         *
         * @note Parsing the URL from string can be expensive. If you need to issue the same request repeatedly, consider manually building
         *       the HTTP request using malloy::http::build_request() and then using either the http_request() overload or the https_request()
         *       functions accepting a pre-parsed HTTP request as a parameter.
         *
         * @param method_ The HTTP method/verb.
         * @param url The URL.
         * @param done Callback invoked on completion. Must satisfy http_callback (@ref client_concepts) with Filter.
         * @param filter Filter to use when parsing the response. Must satisfy response_filter @ref client_concepts.
         *
         * @sa http_request()
         * @sa https_request()
         */
        template<
            malloy::http::concepts::body ReqBody = boost::beast::http::string_body,
            typename Callback,
            concepts::response_filter Filter = detail::default_resp_filter
        >
        requires concepts::http_callback<Callback, Filter>
        [[nodiscard]]
        std::future<malloy::error_code>
        http_request(
            const malloy::http::method method_,
            const std::string_view url,
            Callback&& done,
            Filter filter = {}
        ){
            // Build request
            auto req = malloy::http::build_request<ReqBody>(method_, url);
            if (!req) {
                // ToDo: Here, we'd want to assign a proper error code indicating the actual failure.

                malloy::error_code ec;
                ec.assign(0, boost::beast::generic_category());
                std::promise<malloy::error_code> p;
                p.set_value(std::forward<malloy::error_code>(ec));
                return p.get_future();
            }

            // Make request
#if MALLOY_FEATURE_TLS
            if (req->use_tls())
                return make_http_connection<true>(std::move(*req), std::forward<Callback>(done), std::move(filter));
            else
#endif
                return make_http_connection<false>(std::move(*req), std::forward<Callback>(done), std::move(filter));
        }

#if MALLOY_FEATURE_TLS
        /**
         * Same as http_request() but encrypted with TLS.
         *
         * @sa http_request()
         */
        template<
            malloy::http::concepts::body ReqBody,
            typename Callback,
            concepts::response_filter Filter = detail::default_resp_filter
        >
        requires concepts::http_callback<Callback, Filter>
        [[nodiscard]]
        std::future<malloy::error_code>
        https_request(
            malloy::http::request<ReqBody> req,
            Callback&& done,
            Filter filter = {}
        )
        {
            return make_http_connection<true>(std::move(req), std::forward<Callback>(done), std::move(filter));
        }

        /**
         * @brief Same as ws_connect() but uses TLS
         * @warning tls_init MUST be called before this
         *
         * @sa ws_connect()
         */
        void
        wss_connect(
            const std::string& host,
            std::uint16_t port,
            const std::string& resource,
            std::invocable<malloy::error_code, std::shared_ptr<websocket::connection>> auto&& handler
        )
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
        void
        add_ca_file(const std::filesystem::path& file);

        /**
         * @brief Like add_ca_file(std::filesystem::path) but loads from an in-memory string
         * @warning tls_init() MUST be called before this.
         * @param contents The certificate to be added to the keychain
         *
         * @sa add_ca_file()
         */
        void
        add_ca(const std::string& contents);
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
        void
        ws_connect(
            const std::string& host,
            std::uint16_t port,
            const std::string& resource,
            std::invocable<malloy::error_code, std::shared_ptr<websocket::connection>> auto&& handler
        )
        {
            // Create connection
            make_ws_connection<false>(host, port, resource, std::forward<decltype(handler)>(handler));
        }

    private:
        std::shared_ptr<boost::asio::ssl::context> m_tls_ctx{nullptr};
        std::unique_ptr<boost::asio::io_context> m_ioc_sm{std::make_unique<boost::asio::io_context>()};
        boost::asio::io_context* m_ioc{m_ioc_sm.get()};
        config m_cfg;

        [[nodiscard]]
        friend
        session
        start(controller& ctrl)
        {
            return session{ctrl.m_cfg, ctrl.m_tls_ctx, std::move(ctrl.m_ioc_sm)};
        }

        /**
         * Checks whether the TLS context was initialized.
         * @note This will throw if the TLS context was not initialized.
         */
        void
        check_tls() const
        {
            // Check whether TLS context was initialized
            if (!m_tls_ctx)
                throw std::logic_error("TLS context not initialized.");
        }

        // ToDo: Change this so it returns an awaitable
        template<
            bool isHttps,
            malloy::http::concepts::body Body,
            typename Callback,
            typename Filter
        >
        std::future<malloy::error_code>
        make_http_connection(malloy::http::request<Body>&& req, Callback&& cb, Filter&& filter)
        {
            std::promise<malloy::error_code> prom;
            auto err_channel = prom.get_future();

            // Set User-Agent header if not already set
            if (!malloy::http::has_field(req, malloy::http::field::user_agent)) {
                req.set(malloy::http::field::user_agent, m_cfg.user_agent);
            }

#if MALLOY_FEATURE_TLS
            if constexpr (isHttps) {
                // Create connection
                auto conn = std::make_shared<http::connection_tls<Body, Filter>>(
                    m_cfg.logger->clone(m_cfg.logger->name() + " | HTTPS connection"),
                    *m_ioc,
                    *m_tls_ctx,
                    m_cfg.body_limit
                );

                // Set SNI hostname (many hosts need this to handshake successfully)
                auto ec = conn->set_hostname(req.base()[malloy::http::field::host]);
                if (ec) {
                    prom.set_value(ec);
                    return err_channel;
                }

                // Run
                boost::asio::co_spawn(
                    *m_ioc,
                    conn->run(std::move(req), std::forward<Filter>(filter)),
                    [conn, prom = std::move(prom), cb = std::move(cb)](std::exception_ptr e, http::request_result<Filter> req_result) mutable {    // ToDo: Do we need to capture conn to keep it alive here?!
                        // Note: The order here is important. We need to invoke the callback before we set the promise. Otherwise, a consumer calling get() on the
                        //       promise will get the promise before the consumer callback gets executed. This leads to synchronization problems. The consumer
                        //       expects that the callback gets executed before the error code promise is set (if there is no error).

                        // Handle exceptions
                        if (e)
                            std::rethrow_exception(e);

                        // Invoke callback
                        // Note: Do this BEFORE we set the error code promise (if there's no error)
                        if (!req_result.error_code)
                            cb(std::move(req_result.response));

                        // Set error_code promise
                        prom.set_value(req_result.error_code);
                    }
                );
            }
            else {
#endif
                auto conn = std::make_shared<http::connection_plain<Body, Filter>>(
                    m_cfg.logger->clone(m_cfg.logger->name() + " | HTTP connection"),
                    *m_ioc,
                    m_cfg.body_limit
                );

                // Run
                boost::asio::co_spawn(
                    *m_ioc,
                    conn->run(std::move(req), std::forward<Filter>(filter)),
                    [conn, prom = std::move(prom), cb = std::move(cb)](std::exception_ptr e, http::request_result<Filter> req_result) mutable {    // ToDo: Do we need to capture conn to keep it alive here?!
                        // Note: The order here is important. We need to invoke the callback before we set the promise. Otherwise, a consumer calling get() on the
                        //       promise will get the promise before the consumer callback gets executed. This leads to synchronization problems. The consumer
                        //       expects that the callback gets executed before the error code promise is set (if there is no error).

                        // Handle exceptions
                        if (e)
                            std::rethrow_exception(e);

                        // Invoke callback
                        // Note: Do this BEFORE we set the error code promise (if there's no error)
                        if (!req_result.error_code)
                            cb(std::move(req_result.response));

                        // Set error_code promise
                        prom.set_value(req_result.error_code);
                    }
                );
#if MALLOY_FEATURE_TLS
            }
#endif

            return err_channel;
        }

        template<bool isSecure>
        void
        make_ws_connection(
            const std::string& host,
            std::uint16_t port,
            const std::string& resource,
            std::invocable<malloy::error_code, std::shared_ptr<websocket::connection>> auto&& handler
        )
        {
            // Create connection
            auto resolver = std::make_shared<boost::asio::ip::tcp::resolver>(boost::asio::make_strand(*m_ioc));
            resolver->async_resolve(
                host,
                std::to_string(port),
                [this, resolver, done = std::forward<decltype(handler)>(handler), resource](auto ec, auto results) mutable {
                    if (ec)
                        std::invoke(std::forward<decltype(done)>(done), ec, std::shared_ptr<websocket::connection>{nullptr});
                    else {
                        auto conn = websocket::connection::make(m_cfg.logger->clone("connection"), [this]() -> malloy::websocket::stream {
#if MALLOY_FEATURE_TLS
                            if constexpr (isSecure) {
                                return malloy::websocket::stream{boost::beast::ssl_stream<malloy::tcp::stream<>>{
                                    malloy::tcp::stream<>{boost::asio::make_strand(*m_ioc)}, *m_tls_ctx}};
                            }
                            else
#endif
                                return malloy::websocket::stream{malloy::tcp::stream<>{boost::asio::make_strand(*m_ioc)}};
                        }(), m_cfg.user_agent);

                        conn->connect(results, resource, [conn, done = std::forward<decltype(done)>(done)](auto ec) mutable {
                            if (ec)
                                std::invoke(std::forward<decltype(handler)>(done), ec, std::shared_ptr<websocket::connection>{nullptr});
                            else
                                std::invoke(std::forward<decltype(handler)>(done), ec, conn);
                        });
                    }
                }
            );
        }
    };

    auto start(controller&) -> controller::session;

}   // namespace malloy::client
