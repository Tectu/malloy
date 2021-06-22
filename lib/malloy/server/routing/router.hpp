#pragma once

#include "endpoint_http.hpp"
#include "endpoint_http_regex.hpp"
#include "endpoint_websocket.hpp"
#include "malloy/http/request.hpp"
#include "malloy/http/response.hpp"
#include "malloy/http/http.hpp"
#include "malloy/http/generator.hpp"
#include "malloy/server/http/connection/connection_t.hpp"
#include "malloy/server/http/connection/connection_plain.hpp"
#include "malloy/server/http/connection/connection.hpp"
#include <type_traits>
#include <concepts>


#if MALLOY_FEATURE_TLS
    #include "malloy/server/http/connection/connection_tls.hpp"
#endif

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <spdlog/logger.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <ranges>

namespace spdlog
{
    class logger;
}

namespace malloy::server
{
    namespace detail {
        struct any_callable {
                template<typename T>
                void operator()(T&&) {}
        };
        template<typename T, typename H>
        concept has_handler = requires(T t, H h) { t.set_handler(h); };

        template<typename T, typename... Args>
        concept has_write = requires(T t, Args... args) { t.do_write(std::forward<Args>(args)...); };

        template<typename V>
        concept is_variant = requires(V v) { 
            std::visit(any_callable{}, v); 
        };

        template<typename F>
        concept route_handler =
            std::invocable<F, const malloy::http::request&> ||
            std::invocable<F, const malloy::http::request&,
                           const std::vector<std::string>>;
    }
    // TODO: This might not be thread-safe the way we pass an instance to the listener and then from
    //       there to each session. Investigate and fix this!

    /**
     * An HTTP request router.
     *
     * @details This class implements a basic router for HTTP requests.
     */
    class router
    {
    public:
        /**
         * The method type to use.
         */
        using method_type   = malloy::http::method;

        /**
         * The request type to use.
         */
        using request_type  = malloy::http::request;

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
        explicit router(std::shared_ptr<spdlog::logger> logger);

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
        virtual ~router() = default;

        /**
         * Copy assignment operator.
         *
         * @param rhs The right-hand-side object to copy-assign from.
         * @return A reference to the assignee.
         */
        router& operator=(const router& rhs) = delete;

        /**
         * Move assignment operator.
         *
         * @param rhs The right-hand-side object to move-assign from.
         * @return A reference to the assignee.
         */
        router& operator=(router&& rhs) noexcept = delete;

        /**
         * Set the logger to use.
         *
         * @param logger The logger to use.
         */
        void set_logger(std::shared_ptr<spdlog::logger> logger);

        /**
         * Controls whether the router should automatically generate preflight responses.
         *
         * @note Currently only preflights for routes are generated.
         *
         * @param enabled Whether to enable automatic preflight response generation.
         */
        void generate_preflights(const bool enabled)
        {
            m_generate_preflights = enabled;
        }

        /**
         * Add a sub-router for a specific resource.
         *
         * @param resource The resource base path for the sub-router.
         * @param sub_router The sub-router.
         * @return Whether adding the sub-router was successful.
         */
        bool add_subrouter(std::string resource, std::shared_ptr<router> sub_router);


        /**
         * Add an HTTP regex endpoint.
         *
         * @param method The HTTP method.
         * @param target The resource path (regex).
         * @param handler The handler to generate the response.
         * @return Whether adding the route was successful.
         */
        template<detail::route_handler Func>
        bool add(const method_type method, const std::string_view target, Func&& handler)
        {
            using func_t = std::decay_t<Func>;

            constexpr bool uses_captures =
                std::invocable<func_t, const request_type&,
                               const std::vector<std::string>&>;

            if constexpr (uses_captures) {
                return add_regex_endpoint<
                    uses_captures,
                    std::invoke_result_t<func_t, const request_type&,
                                         const std::vector<std::string>&>>(
                    method, target, std::forward<Func>(handler));
            }
            else {
                return add_regex_endpoint<
                    uses_captures,
                    std::invoke_result_t<func_t, const request_type&>>(
                    method, target, std::forward<Func>(handler));
            }
        }
        /**
         * Add an HTTP file-serving location.
         *
         * @param resource
         * @param storage_base_path
         * @return Whether adding the file serving was successful.
         */
        bool add_file_serving(std::string resource, std::filesystem::path storage_base_path);

        /**
         * Adds an HTTP redirection rule.
         *
         * @param resource_old
         * @param resource_new
         * @param status The HTTP status code to use. This must be a 3xx status code.
         * @return Whether adding the redirect was successful.
         */
        bool add_redirect(malloy::http::status status, std::string&& resource_old, std::string&& resource_new);

        /**
         * Add a websocket endpoint.
         *
         * @param resource The resource path.
         * @param handler The handler for incoming websocket requests.
         * @return Whether adding the endpoint was successful.
         */
        bool add_websocket(std::string resource, malloy::websocket::handler_t handler);

        /**
         * Handle a request.
         *
         * This function will either:
         *   - Forward to the appropriate sub-router
         *   - Handle the HTTP request
         *   - Handle the WS request
         *
         * @tparam isWebsocket
         * @tparam Connection
         * @param doc_root
         * @param req The HTTP request to handle.
         * @param connection The HTTP or WS connection.
         */
        template<
            bool isWebsocket = false
        >
        void handle_request(
            const std::filesystem::path& doc_root,
            malloy::http::request&& req,
            const http::connection_t& connection
        )
        {
            // Check sub-routers
            for (const auto& [resource_base, router] : m_sub_routers) {
                // Check match
                if (!req.uri().resource_starts_with(resource_base))
                    continue;

                // Log
                if (m_logger) {
                    m_logger->debug("invoking sub-router on {}", resource_base);
                }

                // Chop request resource path
                req.uri().chop_resource(resource_base);

                // Let the sub-router handle things from here...
                router->template handle_request<isWebsocket>(doc_root, std::move(req), connection);

                // We're done handling this request
                return;
            }

            if constexpr (isWebsocket)
                handle_ws_request(std::move(req), connection);
            else
                handle_http_request(doc_root, std::move(req), connection);
        }

        /**
         * Handle an HTTP request.
         *
         * @tparam Connection
         * @param doc_root Path to the HTTP document root.
         * @param req The request to handle.
         * @param connection The HTTP connection.
         */
        void handle_http_request(
            const std::filesystem::path& doc_root,
            malloy::http::request&& req,
            const http::connection_t& connection
        )
        {
            // Log
            if (m_logger) {
                m_logger->debug("handling HTTP request: {} {}",
                                req.method_string(),
                                req.uri().resource_string());
            }

            // Check routes
            for (const auto& ep : m_endpoints_http) {
                // Check match
                if (!ep->matches(req))
                    continue;

                // Generate preflight response (if supposed to)
                if (m_generate_preflights && (req.method() == malloy::http::method::options)) {
                    // Log
                    if (m_logger) {
                        m_logger->debug("automatically constructing preflight response.");
                    }

                    // Generate
                    auto resp = generate_preflight_response(req);

                    // Send the response
                    send_response(req, std::move(resp), connection);

                    // We're done handling this request
                    return;
                }

                // Generate the response for the request
                auto resp = ep->handle(req, connection);
                if (resp) {

                    // Send the response
                    send_response(req, std::move(*resp), connection);
                }

                // We're done handling this request
                return;
            }

            // If we end up where we have no meaningful way of handling this request
            return send_response(req, std::move(malloy::http::generator::bad_request("unknown request")), connection);
        }

        /**
         * Handle a WebSocket connection.
         *
         * @tparam Connection The connection type.
         * @param req The original HTTP request that was upgraded.
         * @param connection The WebSocket connection.
         */
        
        void handle_ws_request(
            malloy::http::request&& req,
            http::connection_t connection
        )
        {
            if (m_logger) {
                m_logger->debug("handling WS request: {} {}",
                                req.method_string(),
                                req.uri().resource_string());
            }

            // Check routes
            for (const auto& ep : m_endpoints_websocket) {
                // Check match
                if (ep->resource != req.uri().resource_string())
                    continue;

                // Validate route handler
                if (!ep->handler) {
                    if (m_logger) {
                        m_logger->warn("websocket route with resource path \"{}\" has no valid handler assigned.");
                    }
                    continue;
                }

                // Set handler
                std::visit([&ep](auto& c){ 
                    if constexpr (detail::has_handler<decltype(*c), decltype(ep->handler)>){
                        c->set_handler(ep->handler); 
                    }
                 }, connection);

                // We're done handling this request. The route handler will handle everything from hereon.
                return;
            }
        }

    private:
        std::shared_ptr<spdlog::logger> m_logger;
        std::unordered_map<std::string, std::shared_ptr<router>> m_sub_routers;
        std::vector<std::shared_ptr<endpoint_http>> m_endpoints_http;
        std::vector<std::shared_ptr<endpoint_websocket>> m_endpoints_websocket;
        bool m_generate_preflights = false;

        template<bool UsesCaptures, typename Body, typename Func>
        auto add_regex_endpoint(method_type method, std::string_view target,
                                Func&& handler) -> bool
        {
            // Log
            if (m_logger)
                m_logger->debug("adding route: {}", target);


            // Build regex
            std::regex regex;
            try {
                regex = std::move(std::regex{ target.cbegin(), target.cend() });
            }
            catch (const std::regex_error& e) {
                if (m_logger)
                    m_logger->error("invalid route target supplied \"{}\": {}", target, e.what());
                return false;
            }


            constexpr bool wrapped = detail::is_variant<Body>;
            using bodies_t = std::conditional_t<wrapped, Body, std::variant<Body>>;
            // Build endpoint
            auto ep = std::make_shared<endpoint_http_regex<bodies_t, UsesCaptures>>();
            ep->resource_base = std::move(regex);
            ep->method = method;
            if constexpr (wrapped) {
                ep->handler = std::move(handler);
            } else {
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
                
            ep->writer = [this](const auto& req, auto&& resp, const auto& conn) { 
                    std::visit([&, this](auto&& resp) { send_response(req, std::move(resp), conn);  }, std::move(resp));
            };


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
        bool add_http_endpoint(std::shared_ptr<endpoint_http>&& ep);

        /**
         * Adds a WebSocket endpoint.
         *
         * @note This uses @ref log_or_throw() internally and might therefore throw if no logger is available.
         *
         * @param ep The endpoint to add.
         * @return Whether adding the endpoint was successful.
         */
        bool add_websocket_endpoint(std::shared_ptr<endpoint_websocket>&& ep);

        response_type generate_preflight_response(const request_type& req) const;

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
        bool log_or_throw(const std::exception& exception, const spdlog::level::level_enum level, const FormatString& fmt, Args&&...args)
        {
            if (m_logger) {
                m_logger->log(level, fmt, std::forward<Args>(args)...);
                return false;
            }

            else {
                throw exception;
            }
        }

        /**
         * Send a response.
         *
         * @tparam Connection
         * @param req The request to which we're responding.
         * @param resp The response.
         * @param connection The connection.
         */
        template<typename Body>
        void send_response(const request_type& req, malloy::http::response<Body>&& resp, http::connection_t connection)
        {
            // Add more information to the response
            resp.keep_alive(req.keep_alive());
            resp.version(req.version());
            resp.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
            resp.prepare_payload();

            std::visit([resp = std::move(resp)](auto& c) mutable { 
                if constexpr (detail::has_write<decltype(*c), decltype(resp)>) {
                    c->do_write(std::move(resp)); 
                }
            }, connection);
        }
    };
}
