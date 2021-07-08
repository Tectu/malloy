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
#include "malloy/server/routing/type_traits.hpp"
#include "malloy/type_traits.hpp"
#if MALLOY_FEATURE_TLS
    #include "malloy/server/http/connection/connection_tls.hpp"
#endif

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <spdlog/logger.h>

#include <concepts>
#include <filesystem>
#include <functional>
#include <memory>
#include <vector>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>

namespace spdlog
{
    class logger;
}

namespace malloy::server
{

    namespace detail
    {

        template<typename T, typename... Args>
        concept has_write = requires(T t, Args... args) { t.do_write(std::forward<Args>(args)...); };

        /** 
         * @brief Provides a default Filter to ease use of interface
         * @details A default type for Filter in route::add which allows request
         * filters to be opt-in rather than a required piece of boilerplate
         * @class default_route_handler
         */
        struct default_route_filter {
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
        void send_response(const boost::beast::http::request_header<>& req, malloy::http::response<Body>&& resp, http::connection_t connection)
        {
            // Add more information to the response
            //resp.keep_alive(req.keep_alive); // TODO: Is this needed?, if so its a spanner in the works 
            resp.version(req.version());
            resp.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
            resp.prepare_payload();

            std::visit([resp = std::move(resp)](auto& c) mutable {
                c->do_write(std::move(resp));
            }, connection);
        }
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
        using str_body = boost::beast::http::string_body;

    public:
        template<typename Derived>
        using req_generator = std::shared_ptr<typename http::connection<Derived>::request_generator>;

        using request_header = boost::beast::http::request_header<>;

        /**
         * The method type to use.
         */
        using method_type   = malloy::http::method;

        /**
         * The request type to use.
         */
        using request_type  = malloy::http::request<>;

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
            concepts::route_handler<typename ExtraInfo::request_type> Func
        >
        bool add(const method_type method, const std::string_view target, Func&& handler, ExtraInfo&& extra)
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
                    method, target, std::forward<Func>(handler), std::forward<ExtraInfo>(extra));
            }
            else {
                return add_regex_endpoint<
                    uses_captures,
                    std::invoke_result_t<func_t, const request_type&>
                    >(
                    method, target, std::forward<Func>(handler), std::forward<ExtraInfo>(extra));
            }
        }

        template<concepts::route_handler<typename detail::default_route_filter::request_type> Func>
        auto add(const method_type method, const std::string_view target, Func&& handler) {
            return add(method, target, std::forward<Func>(handler), detail::default_route_filter{});
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
        auto add_websocket(const std::string& resource, typename websocket::connection::handler_t&& handler) {
            // Log
            if (m_logger)
                m_logger->debug("adding websocket endpoint at {}", resource);

            // Check handler
            if (!handler) {
                if (m_logger)
                    m_logger->warn("route has invalid handler. ignoring.");
                return false;
            }

            // Create endpoint
            auto ep = std::make_shared<endpoint_websocket>();
            ep->resource = std::move(resource);
            ep->handler = std::move(handler);

            // Add
            return add_websocket_endpoint(std::move(ep));
        }

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
            bool isWebsocket = false,
            typename Derived,
            typename Connection
        >
        void handle_request(
            const std::filesystem::path& doc_root,
            const req_generator<Derived>& req,
            Connection&& connection,
            malloy::http::uri location
        )
        {
            // Check sub-routers
            for (const auto& [resource_base, router] : m_sub_routers) {
                // Check match
                if (location.resource_starts_with(resource_base))
                    continue;

                // Log
                if (m_logger) {
                    m_logger->debug("invoking sub-router on {}", resource_base);
                }

                // Chop request resource path
                location.chop_resource(resource_base);

                // Let the sub-router handle things from here...
                router->template handle_request<isWebsocket, Derived>(doc_root, std::move(req), connection, location);

                // We're done handling this request
                return;
            }

            if constexpr (isWebsocket)
                handle_ws_request<Derived>(std::move(req), connection, location);
            else
                handle_http_request<Derived>(doc_root, std::move(req), connection, location);
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
        void handle_http_request(
            const std::filesystem::path& doc_root,
            const req_generator<Derived>& req,
            const http::connection_t& connection,
            const malloy::http::uri& location
        )
        {
            // Log
            if (m_logger) {
                m_logger->debug("handling HTTP request: {} {}",
                                req->header().method_string(),
                                req->header().target());
            }

            const auto& header = req->header();
            // Check routes
            for (const auto& ep : m_endpoints_http) {
                // Check match
                if (!ep->matches(header, location))
                    continue;

                // Generate preflight response (if supposed to)
                if (m_generate_preflights && (header.method() == malloy::http::method::options)) {
                    // Log
                    if (m_logger) {
                        m_logger->debug("automatically constructing preflight response.");
                    }
					// Generate
					auto resp = generate_preflight_response(header);

					// Send the response
					detail::send_response(header, std::move(resp), connection);
                    // We're done handling this request
                    return;
                }

                // Generate the response for the request
                auto resp = ep->handle(req, connection);
                if (resp) {

                    // Send the response
                    detail::send_response(req->header(), std::move(*resp), connection);
                }

                // We're done handling this request
                return;
            }

            // If we end up where we have no meaningful way of handling this request
            detail::send_response(req->header(), malloy::http::generator::bad_request("unknown request"), connection);
        }

        /**
         * Handle a WebSocket connection.
         *
         * @tparam Connection The connection type.
         * @param req The original HTTP request that was upgraded.
         * @param connection The WebSocket connection.
         */
        template<typename Derived>
        void handle_ws_request(
            const req_generator<Derived>& gen,
            std::shared_ptr<websocket::connection> connection,
            const malloy::http::uri& location
        )
        {
            m_logger->debug("handling WS request: {} {}",
                gen->header().method_string(),
                location.resource_string()
            );

            // Check routes
            for (const auto& ep : m_endpoints_websocket) {
                // Check match
                if (ep->resource != location.resource_string())
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

    private:
        std::shared_ptr<spdlog::logger> m_logger;
        std::unordered_map<std::string, std::shared_ptr<router>> m_sub_routers;
        std::vector<std::shared_ptr<endpoint_http>> m_endpoints_http;
        std::vector<std::shared_ptr<endpoint_websocket>> m_endpoints_websocket;
        bool m_generate_preflights = false;

        template<
            bool UsesCaptures,
            typename Body,
            concepts::request_filter ExtraInfo,
            typename Func
        >
        auto add_regex_endpoint(method_type method, std::string_view target, Func&& handler, ExtraInfo&& extra) -> bool
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

            constexpr bool wrapped = malloy::concepts::is_variant<Body>;
            using func_t = std::decay_t<Func>;
            using bodies_t = std::conditional_t<wrapped, Body, std::variant<Body>>;

            // Build endpoint
            auto ep = std::make_shared<endpoint_http_regex<bodies_t, std::decay_t<ExtraInfo>, UsesCaptures>>();
            ep->resource_base = std::move(regex);
            ep->method = method;
            ep->filter = std::forward<ExtraInfo>(extra);
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
                    std::visit([&, this](auto&& resp) { detail::send_response(req, std::move(resp), conn);  }, std::move(resp));
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

        /**
         * Generates a preflight response for a specified request.
         *
         * @param req The request header.
         * @return The corresponding response.
         */
        response_type generate_preflight_response(const request_header& req) const;

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
    };

}
