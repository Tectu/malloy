#pragma once

#include "route.hpp"
#include "request.hpp"
#include "response.hpp"
#include "http.hpp"
#include "../utils.hpp"

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

    // TODO: This might be thread-safe the way we pass an instance to the listener and then from
    //       there to each session. Investigate and fix this!

    class router
    {
    public:
        using method_type   = boost::beast::http::verb;
        using request_type  = malloy::http::request;
        using response_type = malloy::http::response;
        using route_type    = route<request_type, response_type>;

        // Construction
        explicit router(std::shared_ptr<spdlog::logger> logger);
        router(const router& other) = delete;
        router(router&& other) = delete;
        virtual ~router() = default;

        // Operators
        router& operator=(const router& rhs) = delete;
        router& operator=(router&& rhs) = delete;

        void add(const method_type method, const std::string_view target, std::function<response_type(const request_type&)>&& handler)
        {
            m_logger->trace("add [route]");

            // Log
            m_logger->debug("adding route: {}", target);

            // Build regex
            std::regex regex;
            try {
                regex = std::move(std::regex{ target.cbegin(), target.cend() });
            }
            catch (const std::regex_error& e) {
                m_logger->error("invalid route target supplied \"{}\": {}", target, e.what());
                return;
            }

            // Build route
            route_type r;
            r.rule = std::move(regex);
            r.verb = method;
            r.handler = std::move(handler);

            // Add route
            try {
                m_routes.emplace_back(std::move(r));
            }
            catch (const std::exception& e) {
                m_logger->critical("could not add route: {}", e.what());
                return;
            }
        }

        void add(std::string resource, std::shared_ptr<router>&& router)
        {
            m_logger->trace("add [router]");

            // Log
            m_logger->debug("adding router: {}", resource);

            // Sanity check target
            {
                if (resource.empty()) {
                    m_logger->error("invalid target \"{}\". not adding router.", resource);
                    return;
                }
            }

            // Sanity check router
            if (not router) {
                m_logger->error("invalid router supplied. not adding router.");
                return;
            }

            // Add router
            try {
                m_routers.try_emplace(std::move(resource), std::move(router));
            }
            catch (const std::exception& e) {
                m_logger->critical("could not add router: {}", e.what());
                return;
            }
        }

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
                send_response(req, response::bad_request("Illegal target requested"), std::move(send));
                return;
            }

            // Check sub-routers
            {
                const std::string_view resource { req.target().data(), req.target().size() };
                for (const auto& [resource_base, router] : m_routers) {
                    // Check if the resource bases matches
                    if (not resource.starts_with(resource_base))
                        continue;

                    // Extract the base from the resource
                    std::string_view router_resource_base = resource.substr(resource_base.size());

                    // Log
                    m_logger->debug("Invoking sub-router with new target base {}", router_resource_base);

                    // Modify the request resource/target
                    req.target( boost::string_view{ router_resource_base.data(), router_resource_base.size() });

                    // Let the sub-router handle things from here...
                    router->handle_request(doc_root, std::move(req), std::move(send));
                    return;
                }
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

            // If we got here we just serve static files
            serve_static_files(doc_root, req, std::forward<Send>(send));
        }

        template<class Send>
        void serve_static_files(
            const std::filesystem::path& doc_root,
            const request& req,
            Send&& send
        )
        {
            m_logger->trace("serve_static_files()");

            // Make sure we can handle the method
            if (req.method() != boost::beast::http::verb::get &&
                req.method() != boost::beast::http::verb::head)
            {
                m_logger->debug("unknown HTTP-method: {}", std::string_view{ req.method_string().data(), req.method_string().size() });
                send_response(req, response::bad_request("Unknown HTTP-method"), std::move(send));
                return;
            }

            // Build the path to the requested file
            std::string path = path_cat(doc_root.string(), req.target());
            if (req.target().back() == '/')
                path.append("index.html");

            // Attempt to open the file
            beast::error_code ec;
            boost::beast::http::file_body::value_type body;
            body.open(path.c_str(), beast::file_mode::scan, ec);

            // Handle the case where the file doesn't exist
            if (ec == beast::errc::no_such_file_or_directory) {
                const std::string_view& target = std::string_view{ req.target().data(), req.target().size() };
                m_logger->debug("target not found: {}", target);
                send_response(req, response::not_found(target), std::move(send));
                return;
            }

            // Handle an unknown error
            if (ec) {
                m_logger->error("unhandled error: {}", ec.message());
                send_response(req, response::server_error(ec.message()), std::move(send));
                return;
            }

            // Cache the size since we need it after the move
            auto const size = body.size();

            // Respond to HEAD request
            if (req.method() == boost::beast::http::verb::head) {
                // Get MIME type
                const std::string_view& mime_type = malloy::mime_type(path);

                // Create response
                response res{ status::ok };
                res.set(boost::beast::http::field::content_type, boost::string_view{ mime_type.data(), mime_type.size() });
                res.content_length(size);

                return send_response(req, std::move(res), std::move(send));
            }

            // Respond to GET request
            auto res = response::file(path);

            return send_response(req, std::move(res), std::move(send));
        }

    private:
        std::shared_ptr<spdlog::logger> m_logger;
        std::vector<route_type> m_routes;
        std::unordered_map<std::string, std::shared_ptr<router>> m_routers;

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

        // Append an HTTP rel-path to a local filesystem path.
        // The returned path is normalized for the platform.
        [[nodiscard]]
        static
        std::string
        path_cat(beast::string_view base, beast::string_view path);
    };
}
