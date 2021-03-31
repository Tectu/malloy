#pragma once

#include "route.hpp"
#include "request.hpp"
#include "response.hpp"
#include "http.hpp"

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

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace http = beast::http;                   // from <boost/beast/http.hpp>

namespace spdlog
{
    class logger;
}

namespace malloy::http::server
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
        enum class redirection
        {
            permanent,
            temporarily
        };

        using method_type   = boost::beast::http::verb;
        using request_type  = malloy::http::request;
        using response_type = malloy::http::response;
        using route_type    = route<request_type, response_type>;

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
         * Add a handler for a specific method & resource.
         *
         * @param method
         * @param target
         * @param handler
         */
        void add(method_type method, std::string_view target, std::function<response_type(const request_type&)>&& handler);

        /**
         * Add a sub-router for a specific resource.
         *
         * @param resource
         * @param router
         */
        void add(std::string resource, std::shared_ptr<router>&& router);

        /**
         * Add a file-serving location.
         *
         * @param resource
         * @param storage_base_path
         */
        void add_file_serving(std::string resource, std::filesystem::path storage_base_path);

        /**
         * Adds a redirection rule.
         *
         * @param resource_old
         * @param resource_new
         * @param permanent Returns HTTP status 301 if true, 302 otherwise.
         */
        void add_redirect(enum redirection type, std::string resource_old, std::string resource_new);

        /**
         * Handle a request.
         *
         * @tparam Send The response writer type.
         * @param doc_root Path to the HTTP document root.
         * @param req The request to handle.
         * @param send The response writer to use.
         */
        template<class Send>
        void handle_request(
            const std::filesystem::path& doc_root,
            request&& req,
            Send&& send
        )
        {
            m_logger->trace("handle_request()");

            // Log
            m_logger->debug("handling request: {} {}",
                std::string_view{ req.method_string().data(), req.method_string().size() },
                std::string_view{ req.target().data(), req.target().size() }
            );

            // Request path must be absolute and not contain "..".
            if (req.target().empty() ||
                req.target()[0] != '/' ||
                req.target().find("..") != beast::string_view::npos)
            {
                m_logger->debug("illegal request-target.");
                send_response(req, response::bad_request("Illegal target requested"), std::forward<Send>(send));
                return;
            }

            // Check redirects
            {
                for (const auto& record : m_redirects) {
                    // Check if the resource matches
                    // ToDo: Use req.uri() here instead!
                    if (record.resource_old not_eq std::string_view{ req.target().data(), req.target().size() })
                        continue;

                    // Log
                    m_logger->debug("found matching redirection record: {} -> {}", record.resource_old, record.resource_new);

                    // Create the response
                    http::status status = status::internal_server_error;
                    if (record.type == redirection::permanent)
                        status = status::permanent_redirect;
                    else if (record.type == redirection::temporarily)
                        status = status::temporary_redirect;
                    response resp{ status };
                    resp.set("Location", record.resource_new);

                    // Send the response
                    send_response(req, std::move(resp), std::forward<Send>(send));

                    // We're done with handling this request
                    return;
                }
            }

            // Check sub-routers
            {
#warning "ToDo: Use uri::resource() instead."
                const std::string_view resource { req.target().data(), req.target().size() };
                for (const auto& [resource_base, router] : m_routers) {
                    // Check if the resource bases matches
                    if (not resource.starts_with(resource_base))
                        continue;

                    // Extract the base from the resource
                    std::string_view router_resource_base = resource.substr(resource_base.size());

                    // Log
                    m_logger->debug("invoking sub-router with new target base {}", router_resource_base);

                    // Modify the request resource/target
                    req.target( boost::string_view{ router_resource_base.data(), router_resource_base.size() });

                    // Let the sub-router handle things from here...
                    router->handle_request(doc_root, std::move(req), std::forward<Send>(send));

                    // We're done handling this request
                    return;
                }
            }

            // Check file servings
            for (const auto& [resource_base, storage_location_base] : m_file_servings) {
                // Alias
                const std::string_view& req_resource = req.uri().resource_string();
                const auto& resources = req.uri().resource();

                // Check match
                if (not req_resource.starts_with(resource_base))
                    continue;

                // Extract the base from the resource
                std::string_view adjusted_resource_base = req_resource.substr(resource_base.size());

                // Disallow relative paths
                if (adjusted_resource_base.find("..") != std::string_view::npos) {
                    m_logger->warn("received request containing relative path: {}", adjusted_resource_base);
                }

                ///////////////
                /// DO NOT DO THIS!!!!
                ///
                /// instead: Sanitize the path properly.
                ///          Also respond invalid request when path contains '..'
                /////////////
                if (adjusted_resource_base.starts_with('/'))
                    adjusted_resource_base = adjusted_resource_base.substr(1);

                // Assemble path
                const std::filesystem::path& path = storage_location_base / adjusted_resource_base;

                // Log
                m_logger->debug("serving static file {} --> {}", req_resource, path.string());

                // Create response
                auto resp = response::file(path);

                // Send response
                send_response(req, std::move(resp), std::forward<Send>(send));

                // We're done handling this request
                return;
            }

            // Check routes
            for (const auto& route : m_routes) {
                // Check if this is a preflight request
                if (req.method() ==
                    boost::beast::http::verb::options) {
                    m_logger->debug("automatically constructing preflight response.");

                    // Methods
                    std::vector<std::string> method_strings;
                    for (const auto& route : m_routes) {
                        if (not route.matches_target(std::string{
                            req.target().data(),
                            req.target().size()}))
                            continue;
                        method_strings.emplace_back(boost::beast::http::to_string(route.verb));
                    }
                    std::string methods_string;
                    for (const auto &str : method_strings) {
                        methods_string += str;
                        if (&str not_eq
                            &method_strings.back())
                            methods_string += ", ";
                    }
                    m_logger->debug("preflight allowed methods: {}", methods_string);

                    response res{ status::ok };
                    res.set(boost::beast::http::field::content_type, "text/html");
                    res.base().set("Access-Control-Allow-Origin", "http://127.0.0.1:8080");
                    res.base().set("Access-Control-Allow-Methods", methods_string);
                    res.base().set("Access-Control-Allow-Headers", "Content-Type");
                    res.base().set("Access-Control-Max-Age", "60");

                    m_logger->debug("sending preflight response.");

                    // Send the response
                    send_response(req, std::move(res), std::move(send));
                    return;
                }

                // Handle the route
                if (route.matches_request(req)) {
                    m_logger->debug("route matching!");

                    // Check handler validity
                    if (not route.handler) {
                        m_logger->critical("route has no valid handler.");
                        break;
                    }

                    auto resp = route.handler(req);
                    send_response(req, std::move(resp), std::move(send));

                    return;
                }
            }
        }

    private:
        /**
         * A record representing a redirection.
         */
        struct redirection_record
        {
            enum redirection type;
            std::string resource_old;
            std::string resource_new;
        };

        std::shared_ptr<spdlog::logger> m_logger;
        std::vector<redirection_record> m_redirects;
        std::vector<route_type> m_routes;
        std::unordered_map<std::string, std::shared_ptr<router>> m_routers;
        std::unordered_map<std::string, std::filesystem::path> m_file_servings;

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
