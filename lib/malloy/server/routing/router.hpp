#pragma once

#include "route.hpp"
#include "malloy/http/request.hpp"
#include "malloy/http/response.hpp"
#include "malloy/http/http.hpp"
#include "malloy/http/generator.hpp"

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
        using response_type = malloy::http::response;

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
         * Add a handler for a specific method & resource.
         *
         * @param method The HTTP method.
         * @param target The resource path.
         * @param handler The handler to generate the response.
         * @return Whether adding the route was successful.
         */
        bool add(method_type method, std::string_view target, std::function<response_type(const request_type&)>&& handler);

        /**
         * Add a sub-router for a specific resource.
         *
         * @param resource The resource base path for the sub-router.
         * @param sub_router The sub-router.
         * @return Whether adding the sub-router was successful.
         */
        bool add_subrouter(std::string resource, std::shared_ptr<router> sub_router);

        /**
         * Add a file-serving location.
         *
         * @param resource
         * @param storage_base_path
         * @return Whether adding the file serving was successful.
         */
        bool add_file_serving(std::string resource, std::filesystem::path storage_base_path);

        /**
         * Adds a redirection rule.
         *
         * @param resource_old
         * @param resource_new
         * @param status The HTTP status code to use. This must be a 3xx status code.
         * @return Whether adding the redirect was successful.
         */
        bool add_redirect(malloy::http::status status, std::string&& resource_old, std::string&& resource_new);

        /**
         * Handle a request.
         *
         * This function will either:
         *   - Forward to the appropriate sub-router
         *   - Handle the HTTP request
         *   - Handle the WS request
         *
         * @tparam isWebsocket
         * @tparam Send
         * @param doc_root
         * @param req
         * @param send
         */
        template<
            bool isWebsocket = false,
            class Send
        >
        void handle_request(
            const std::filesystem::path& doc_root,
            malloy::http::request&& req,
            Send&& send
        )
        {
            // Check sub-routers
            for (const auto& [resource_base, router] : m_sub_routers) {
                // Check match
                if (not req.uri().resource_starts_with(resource_base))
                    continue;

                // Log
                m_logger->debug("invoking sub-router on {}", resource_base);

                // Chop request resource path
                req.uri().chop_resource(resource_base);

                // Let the sub-router handle things from here...
                router->template handle_request<isWebsocket>(doc_root, std::move(req), std::forward<Send>(send));

                // We're done handling this request
                return;
            }

            if constexpr (isWebsocket)
                handle_ws_request(std::move(req), std::forward<Send>(send));
            else
                handle_http_request(doc_root, std::move(req), std::forward<Send>(send));
        }

        /**
         * Handle an HTTP request.
         *
         * @tparam Send The response writer type.
         * @param doc_root Path to the HTTP document root.
         * @param req The request to handle.
         * @param send The response writer to use.
         */
        template<class Send>
        void handle_http_request(
            const std::filesystem::path& doc_root,
            malloy::http::request&& req,
            Send&& send
        )
        {
            // Log
            m_logger->debug("handling HTTP request: {} {}",
                std::string_view{ req.method_string().data(), req.method_string().size() },
                req.uri().resource_string()
            );

            // Check routes
            for (const auto& route : m_routes) {
                // Check match
                if (not route->matches(req))
                    continue;

                // Generate preflight response (if supposed to)
                if (m_generate_preflights and (req.method() == malloy::http::method::options)) {
                    // Log
                    m_logger->debug("automatically constructing preflight response.");

                    // Generate
                    auto resp = generate_preflight_response(req);

                    // Send the response
                    send_response(req, std::move(resp), std::forward<Send>(send));

                    // We're done handling this request
                    return;
                }

                // Generate the response for the request
                auto resp = route->handle(req);

                // Send the response
                send_response(req, std::move(resp), std::forward<Send>(send));

                // We're done handling this request
                return;
            }

            // If we end up where we have no meaningful way of handling this request
            return send_response(req, std::move(malloy::http::generator::bad_request("unknown request")), std::forward<Send>(send));
        }

        template<
            class Send
        >
        void handle_ws_request(
            malloy::http::request&& req,
            Send&& send
        )
        {
            m_logger->debug("handling WS request: {} {}",
                std::string_view{ req.method_string().data(), req.method_string().size() },
                req.uri().resource_string()
            );

            // Echo the message
            send("Hello WS!");
        }

    private:
        std::shared_ptr<spdlog::logger> m_logger;
        std::unordered_map<std::string, std::shared_ptr<router>> m_sub_routers;
        std::vector<std::shared_ptr<route<request_type, response_type>>> m_routes;
        bool m_generate_preflights = false;

        /**
         * Adds a route to the list of routes.
         *
         * @note This uses @ref log_or_throw() internally and might therefore throw if no logger is available.
         *
         * @param r The route to add.
         * @return Whether adding the route was successful.
         */
        bool add_route(std::shared_ptr<route<request_type, response_type>>&& r);

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
         * @tparam Send The response writer type.
         * @param req The request to which we're responding.
         * @param resp The response.
         * @param send The response writer.
         */
        template<typename Send>
        void send_response(const request_type& req, response_type&& resp, Send&& send)
        {
            // Add more information to the response
            resp.keep_alive(req.keep_alive());
            resp.version(req.version());
            resp.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
            resp.prepare_payload();

            send(std::move(resp));
        }
    };
}
