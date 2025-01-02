#pragma once

#include "endpoint_http.hpp"
#include "endpoint_http_regex.hpp"
#include "endpoint_http_files.hpp"
#include "endpoint_websocket.hpp"
#include "type_traits.hpp"
#include "../http/connection.hpp"
#include "../http/connection_plain.hpp"
#include "../http/connection_t.hpp"
#include "../http/preflight_config.hpp"
#include "../../core/type_traits.hpp"
#include "../../core/detail/version_checks.hpp"
#include "../../core/http/generator.hpp"
#include "../../core/http/http.hpp"
#include "../../core/http/request.hpp"
#include "../../core/http/response.hpp"
#include "../../core/http/utils.hpp"
#if MALLOY_FEATURE_TLS
    #include "../http/connection_tls.hpp"
#endif

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <spdlog/logger.h>

#include <concepts>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace spdlog
{
    class logger;
}

namespace malloy::server
{
    class routing_context;

    namespace detail
    {

        template<typename T, typename... Args>
        concept has_write = requires(T t, Args... args)
        {
            t.do_write(std::forward<Args>(args)...);
        };

        /**
         * @brief Provides a default Filter to ease use of interface
         * @details A default type for Filter in route::add which allows request
         * filters to be opt-in rather than a required piece of boilerplate
         * @class default_route_handler
         */
        struct default_route_filter
        {
            using request_type = malloy::http::request<boost::beast::http::string_body>;
            using header_type = boost::beast::http::request_header<>;

            void setup_body(const header_type&, typename request_type::body_type::value_type&) const {}
        };
        static_assert(concepts::request_filter<default_route_filter>, "Default handler must satisfy route filter");

        /**
         * Send a response.
         *
         * @tparam Connection
         * @param req The request to which we're responding.
         * @param resp The response.
         * @param connection The connection.
         */
        template<typename Body>
        void
        send_response(const boost::beast::http::request_header<>& req, malloy::http::response<Body>&& resp, http::connection_t connection, std::string_view server_str)
        {
            // Add more information to the response
            //resp.keep_alive(req.keep_alive); // TODO: Is this needed?, if so its a spanner in the works
            resp.version(req.version());
            if (!malloy::http::has_field(resp, malloy::http::field::server))
                resp.set(malloy::http::field::server, server_str);
            resp.prepare_payload();

            std::visit(
                [resp = std::move(resp)](auto& c) mutable {
                    c->do_write(std::move(resp));
                },
                connection
            );
        }
    }    // namespace detail

    /**
     * An HTTP request router.
     *
     * @details This class implements a basic router for HTTP requests.
     */
    class router final
    {
        /**
         * Create the lambda wrapped callback for the writer
         */
        auto
        make_endpt_writer_callback()
        {
            return [this]<typename R>(const auto& req, R&& resp, const auto& conn) {
                std::visit(
                    [&, this]<typename Re>(Re&& resp) {
                        detail::send_response(req, std::forward<Re>(resp), conn, m_server_str);
                    },
                    std::forward<R>(resp)
                );
            };
        }

        class abstract_req_validator
        {
        public:
            virtual
            ~abstract_req_validator() = default;

            virtual
            bool
            process(const boost::beast::http::request_header<>&, const http::connection_t& conn) = 0;
        };

        template<concepts::request_validator V, typename Writer>
        class req_validator_impl :
            public abstract_req_validator
        {
        public:
            Writer writer;

            req_validator_impl(V validator, Writer writer_) :
                writer{std::move(writer_)},
                m_validator{std::move(validator)}
            {
            }

            bool
            process(const boost::beast::http::request_header<>& h, const http::connection_t& conn) override
            {
                auto maybe_resp = std::invoke(m_validator, h);
                if (!maybe_resp)
                    return false;
                else {
                    writer(h, std::move(*maybe_resp), conn);
                    return true;
                }
            }

        private:
            V m_validator;
        };

        class policy_store
        {
        public:
            policy_store(std::string reg, std::unique_ptr<abstract_req_validator> validator) :
                m_validator{std::move(validator)},
                m_raw_reg{std::move(reg)}
            {
            }

            [[nodiscard]]
            bool
            process(const boost::beast::http::request_header<>& h, const http::connection_t& conn) const
            {
                if (!matches(h.target()))
                    return false;
                else
                    return m_validator->process(h, conn);
            }

        private:
            bool
            matches(std::string_view url) const
            {
                if (!m_compiled_reg)
                    compile_match_expr();
                std::string surl{url.begin(), url.end()}; // Must be null terminated
                return std::regex_match(surl, *m_compiled_reg);
            }

            void
            compile_match_expr() const
            {
                m_compiled_reg = std::regex{m_raw_reg};
            }

            std::unique_ptr<abstract_req_validator> m_validator;
            mutable std::optional<std::regex> m_compiled_reg;
            std::string m_raw_reg;
        };

    public:
        template<typename Derived>
        using req_generator = std::shared_ptr<typename http::connection<Derived>::request_generator>;

        using request_header = boost::beast::http::request_header<>;

        /**
         * The method type to use.
         */
        using method_type = malloy::http::method;

        /**
         * The request type to use.
         */
        using request_type = malloy::http::request<>;

        /**
         * The response type to use.
         */
        using response_type = malloy::http::response<>;

        /**
         * Default constructor.
         */
        router() = default;

        /**
         * Constructor.
         *
         * @param logger The logger instance to use.
         */
        explicit
        router(std::shared_ptr<spdlog::logger> logger);

        /**
         * Copy constructor.
         */
        router(const router& other) = delete;

        /**
         * Move constructor.
         */
        router(router&& other) noexcept = delete;

        /**
         * Destructor.
         */
        ~router() = default;

        /**
         * Copy assignment operator.
         *
         * @param rhs The right-hand-side object to copy-assign from.
         * @return A reference to the assignee.
         */
        router&
        operator=(const router& rhs) = delete;

        /**
         * Move assignment operator.
         *
         * @param rhs The right-hand-side object to move-assign from.
         * @return A reference to the assignee.
         */
        router&
        operator=(router&& rhs) noexcept = delete;

        /**
         * Set the logger to use.
         *
         * @param logger The logger to use.
         */
        void
        set_logger(std::shared_ptr<spdlog::logger> logger);

        /**
         * Add a sub-router for a specific resource.
         *
         * @param resource The resource base path for the sub-router.
         * @param sub_router The sub-router.
         * @return Whether adding the sub-router was successful.
         */
        bool
        add_subrouter(std::string resource, std::unique_ptr<router> sub_router);

        /**
         * Add an HTTP regex endpoint.
         *
         * @tparam ExtraInfo a type that satisfies request_filter. Used to provide additional customisation of request handling. 
         *  Must satisfy request_filter @ref route_concepts
         * @tparam Func invoked on a request to the specified target with the specified method. 
         *  Must satisfy route_handler @ref route_concepts
         * @param method The HTTP method.
         * @param target The resource path (regex).
         * @param handler The handler to generate the response.
         * @return Whether adding the route was successful.
         */
        template<
            concepts::request_filter ExtraInfo,
            concepts::route_handler<typename ExtraInfo::request_type> Func>
        bool
        add(const method_type method, const std::string_view target, Func&& handler, ExtraInfo&& extra)
        {
            using func_t = std::decay_t<Func>;

            constexpr bool uses_captures = std::invocable<func_t, const request_type&, const std::vector<std::string>&>;

            if constexpr (uses_captures) {
                return add_regex_endpoint<
                        uses_captures,
                        std::invoke_result_t<func_t, const request_type&, const std::vector<std::string>&>
                    >(
                        method, target, std::forward<Func>(handler), std::forward<ExtraInfo>(extra)
                    );
            }
            else {
                return add_regex_endpoint<
                        uses_captures,
                        std::invoke_result_t<func_t, const request_type&>
                    >(
                        method, target, std::forward<Func>(handler), std::forward<ExtraInfo>(extra)
                    );
            }
        }

        template<concepts::route_handler<typename detail::default_route_filter::request_type> Func>
        auto
        add(const method_type method, const std::string_view target, Func&& handler)
        {
            return add(method, target, std::forward<Func>(handler), detail::default_route_filter{});
        }

        bool
        add_preflight(std::string_view target, http::preflight_config cfg);

        /**
         * Add an HTTP file-servinc location.
         *
         * @tparam CacheControl
         * @param resource
         * @param storage_base_path
         * @param cc
         * @return Whether adding the file serving was successful.
         */
        template<malloy::concepts::callable_string CacheControl>
        bool
        add_file_serving(std::string resource, std::filesystem::path storage_base_path, const CacheControl& cc)
        {
            // Log
            if (m_logger)
                m_logger->trace("adding file serving location: {} -> {}", resource, storage_base_path.string());

            // Create endpoint
            auto ep = std::make_unique<endpoint_http_files>();
            ep->resource_base = resource;
            ep->base_path     = std::move(storage_base_path);
            ep->cache_control = cc();
            ep->writer        = make_endpt_writer_callback();

            // Add
            return add_http_endpoint(std::move(ep));
        }

        /**
         * Add an HTTP file-serving location.
         *
         * @param resource
         * @param storage_base_path
         * @return Whether adding the file serving was successful.
         */
        bool
        add_file_serving(std::string resource, std::filesystem::path storage_base_path)
        {
            return add_file_serving(
                std::move(resource),
                std::move(storage_base_path),
                []() -> std::string { return ""; }
            );
        }

        /**
         * Adds an HTTP redirection rule.
         *
         * @param resource_old
         * @param resource_new
         * @param status The HTTP status code to use. This must be a 3xx status code.
         * @return Whether adding the redirect was successful.
         */
        bool
        add_redirect(malloy::http::status status, std::string&& resource_old, std::string&& resource_new);

        /**
         * Add a websocket endpoint.
         *
         * @param resource The resource path.
         * @param handler The handler for incoming websocket requests.
         * @return Whether adding the endpoint was successful.
         */
        bool
        add_websocket(std::string&& resource, typename websocket::connection::handler_t&& handler);

        /**
         * Add an access policy for a specific resource.
         *
         * A policy allow restricting access to any resource registered on this router.
         *
         * @param resource The resource path.
         * @param policy The policy.
         */
        template<concepts::request_validator Policy>
        void
        add_policy(const std::string& resource, Policy&& policy)
        {
            if (m_logger)
                m_logger->trace("adding policy: {}", resource);

            using policy_t = std::decay_t<Policy>;
            auto writer = [this](const auto& header, auto&& resp, auto&& conn) { detail::send_response(header, std::forward<decltype(resp)>(resp), std::forward<decltype(conn)>(conn), m_server_str); };

            m_policies.emplace_back(resource, std::make_unique<req_validator_impl<policy_t, decltype(writer)>>(std::forward<Policy>(policy), std::move(writer)));
        }

        /**
         * Handle a request.
         *
         * This function will either:
         *   - Forward to the appropriate sub-router
         *   - Handle the HTTP request
         *   - Handle the WS request
         *
         * HTTP requests are first policy checked.
         *
         * @tparam isWebsocket
         * @tparam Connection
         * @param doc_root
         * @param req The HTTP request to handle.
         * @param connection The HTTP or WS connection.
         */
        template<
            bool isWebsocket = false,
            typename Derived,
            typename Connection>
        void
        handle_request(
            const std::filesystem::path& doc_root,
            const req_generator<Derived>& req,
            Connection&& connection
        )
        {
            // Handle policy
            if constexpr (!isWebsocket) {
                if (is_handled_by_policies<Derived>(req, connection))
                    return;
            }

            // Check sub-routers
            for (const auto& [resource_base, router] : m_sub_routers) {
                // Check match
                const auto res_str = malloy::http::resource_string(req->header());
                if (!res_str.starts_with(resource_base))
                    continue;

                // Chop request resource path
                malloy::http::chop_resource(req->header(), resource_base);

                // Let the sub-router handle things from here...
                router->template handle_request<isWebsocket, Derived>(doc_root, std::move(req), connection);

                // We're done handling this request
                return;
            }

            //
            // At this point we know that this particular router is going to handle the request.
            //

            // Forward to appropriate handler.
            if constexpr (isWebsocket)
                handle_ws_request<Derived>(std::move(req), connection);
            else
                handle_http_request<Derived>(doc_root, std::move(req), connection);
        }

        constexpr
        std::string_view
        server_string() const
        {
            return m_server_str;
        }

    private:
        std::shared_ptr<spdlog::logger> m_logger{nullptr};
        std::unordered_map<std::string, std::unique_ptr<router>> m_sub_routers;
        std::vector<std::unique_ptr<endpoint_http>> m_endpoints_http;
        std::vector<std::unique_ptr<endpoint_websocket>> m_endpoints_websocket;
        std::vector<policy_store> m_policies;                           // Access policies for resources
        std::string_view m_server_str;

        friend class routing_context;

        router(std::shared_ptr<spdlog::logger> logger, std::string_view m_server_str);

        void
        set_server_string(std::string_view str);

        template<typename Derived>
        [[nodiscard]]
        bool
        is_handled_by_policies(const req_generator<Derived>& req, const http::connection_t& connection)
        {
            return std::any_of(
                std::cbegin(m_policies),
                std::cend(m_policies),
                [&](const policy_store& policy) {
                    return policy.process(req->header(), connection);
            });
        }

        /**
         * Handle an HTTP request.
         *
         * @tparam Connection
         * @param doc_root Path to the HTTP document root.
         * @param req The request to handle.
         * @param connection The HTTP connection.
         */
        template<typename Derived>
        void
        handle_http_request(
            const std::filesystem::path&,
            const req_generator<Derived>& req,
            const http::connection_t& connection
        )
        {
            // Log
            if (m_logger) {
                m_logger->trace("handling HTTP request: {} {}",
                    std::string_view{req->header().method_string()},
                    std::string_view{req->header().target()}
                );
            }

            const auto& header = req->header();

            // Check routes
            for (const auto& ep : m_endpoints_http) {
                // Check match
                if (!ep->matches(header))
                    continue;

                // Generate the response for the request
                auto resp = ep->handle(req, connection);
                if (resp) {
                    // Send the response
                    detail::send_response(req->header(), std::move(*resp), connection, m_server_str);
                }

                // We're done handling this request
                return;
            }

            // If we end up where we have no meaningful way of handling this request
            detail::send_response(req->header(), malloy::http::generator::bad_request("unknown request"), connection, m_server_str);
        }

        /**
         * Handle a WebSocket connection.
         *
         * @tparam Connection The connection type.
         * @param req The original HTTP request that was upgraded.
         * @param connection The WebSocket connection.
         */
        template<typename Derived>
        void
        handle_ws_request(
            const req_generator<Derived>& gen,
            const std::shared_ptr<websocket::connection>& connection
        )
        {
            const auto res_string = malloy::http::resource_string(gen->header());
            m_logger->trace("handling WS request: {} {}",
                std::string_view{gen->header().method_string()},
                res_string
            );

            // Check routes
            for (const auto& ep : m_endpoints_websocket) {
                // Check match
                if (ep->resource != res_string)
                    continue;

                // Validate route handler
                if (!ep->handler) {
                    m_logger->warn("websocket route with resource path \"{}\" has no valid handler assigned.");
                    continue;
                }

                malloy::http::request req;
                req.base() = gen->header();
                ep->handler(std::move(req), connection);

                // We're done handling this request. The route handler will handle everything from hereon.
                return;
            }
        }

        template<
            bool UsesCaptures,
            typename Body,
            concepts::request_filter ExtraInfo,
            typename Func>
        bool
        add_regex_endpoint(method_type method, std::string_view target, Func&& handler, ExtraInfo&& extra)
        {
            // Log
            if (m_logger)
                m_logger->trace("adding route: {}", target);

            // Build regex
            std::regex regex;
            try {
                regex = std::regex{target.cbegin(), target.cend()};
            }
            catch (const std::regex_error& e) {
                if (m_logger)
                    m_logger->error("invalid route target supplied \"{}\": {}", target, e.what());
                return false;
            }

            constexpr bool wrapped = malloy::concepts::is_variant<Body>;
            using bodies_t = std::conditional_t<wrapped, Body, std::variant<Body>>;

            // Build endpoint
            auto ep = std::make_unique<endpoint_http_regex<bodies_t, std::decay_t<ExtraInfo>, UsesCaptures>>();
            ep->resource_base = std::move(regex);
            ep->method = method;
            ep->filter = std::forward<ExtraInfo>(extra);
            if constexpr (wrapped) {
                ep->handler = std::move(handler);
            }
            else {
                ep->handler =
                    [w = std::forward<Func>(handler)](auto&&... args) {
                        return std::variant<Body>{w(std::forward<decltype(args)>(args)...)};
                    };
            }

            // Check handler
            if (!ep->handler) {
                if (m_logger)
                    m_logger->warn("route has invalid handler. ignoring.");
                return false;
            }

            ep->writer = make_endpt_writer_callback();

            // Add route
            return add_http_endpoint(std::move(ep));
        }

        /**
         * Adds an HTTP endpoint.
         *
         * @note This uses @ref log_or_throw() internally and might therefore throw if no logger is available.
         *
         * @param ep The endpoint to add.
         * @return Whether adding the endpoint was successful.
         */
        bool
        add_http_endpoint(std::unique_ptr<endpoint_http>&& ep);

        /**
         * Adds a WebSocket endpoint.
         *
         * @note This uses @ref log_or_throw() internally and might therefore throw if no logger is available.
         *
         * @param ep The endpoint to add.
         * @return Whether adding the endpoint was successful.
         */
        bool
        add_websocket_endpoint(std::unique_ptr<endpoint_websocket>&& ep);

        /**
         * Adds a message to the log or throws an exception if no logger is available.
         *
         * @tparam FormatString
         * @tparam Args
         * @param exception
         * @param level
         * @param fmt
         * @param args
         * @return
         */
        template<typename FormatString, typename... Args>
        bool
        log_or_throw(
            const std::exception& exception,
            const spdlog::level::level_enum level,
            const FormatString& fmt,
            Args&&... args
        )
        {
            if (m_logger) {
                m_logger->log(level,
#if MALLOY_DETAIL_HAS_FMT_8
                      fmt::runtime(fmt)
#else
                      fmt
#endif
                      ,
                      std::forward<Args>(args)...
                );
                return false;
            }

            else
                throw exception;
        }
    };

}    // namespace malloy::server
