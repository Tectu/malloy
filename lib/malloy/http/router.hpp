#pragma once

#include "route.hpp"
#include "response.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <spdlog/logger.h>

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

namespace malloy::server::http
{

    // TODO: This might be thread-safe the way we pass an instance to the listener and then from
    //       there to each session. Investigate and fix this!

    class router
    {
    public:
        using method_type  = boost::beast::http::verb;
        using response_type = boost::beast::http::message<false, boost::beast::http::string_body, boost::beast::http::basic_fields<std::allocator<char>>>;
        using handler_type = std::function<response_type(const boost::beast::http::request_parser<boost::beast::http::string_body>::value_type&)>;
        using route_type   = route<handler_type>;

        // Construction
        explicit router(std::shared_ptr<spdlog::logger> logger);
        router(const router& other) = delete;
        router(router&& other) = delete;
        virtual ~router() = default;

        // Operators
        router& operator=(const router& rhs) = delete;
        router& operator=(router&& rhs) = delete;

        // General
        void add(const method_type method, const std::string_view target, handler_type&& on_request)
        {
            m_logger->trace("add_route()");

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
            r.handler = std::move(on_request);

            // Add route
            try {
                m_routes.emplace_back(std::move(r));
            }
            catch (const std::exception& e) {
                m_logger->critical("could not add route: {}", e.what());
                return;
            }
        }


        template<
            class Body, class Allocator,
            class Send
        >
        void handle_request(
            beast::string_view doc_root,
            boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>&& req,
            Send&& send
        )
        {
            m_logger->trace("handle_request()");

            // Log
            const boost::string_view &verb_str = boost::beast::http::to_string(req.method());
            m_logger->debug("handling request:\n  verb: {}\n  uri : {}",
                            verb_str.data(),
                            std::string_view{
                                req.target().data(),
                                req.target().size()}
            );

            // Request path must be absolute and not contain "..".
            if (req.target().empty() ||
                req.target()[0] !=
                '/' ||
                req.target().find("..") !=
                beast::string_view::npos) {
                m_logger->debug("illegal request-target.");
                send(bad_request(req, "Illegal request-target"));
                return;
            }

            // Check routes
            for (const auto &route : m_routes) {
                // Check if this is a preflight request
                if (req.method() ==
                    boost::beast::http::verb::options) {
                    m_logger->debug("automatically constructing preflight response.");

                    // Methods
                    std::vector<std::string> method_strings;
                    for (const auto &route : m_routes) {
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

                    boost::beast::http::response<boost::beast::http::string_body> res{
                        boost::beast::http::status::ok,
                        req.version()};
                    res.set(boost::beast::http::field::content_type, "text/html");
                    res.keep_alive(req.keep_alive());
                    res.base().set("Access-Control-Allow-Origin", "http://127.0.0.1:8080");
                    res.base().set("Access-Control-Allow-Methods", methods_string);
                    res.base().set("Access-Control-Allow-Headers", "Content-Type");
                    res.base().set("Access-Control-Max-Age", "60");
                    res.prepare_payload();

                    m_logger->debug("sending preflight response.");

                    // Send the response
                    send(std::move(res));
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
                    resp.keep_alive(req.keep_alive());
                    resp.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
                    send(std::move(resp));

                    return;
                }
            }

            // If we got here we just serve static files
            serve_static_files(doc_root, std::move(req), std::forward<Send>(send));
        }

        template<
            class Body, class Allocator,
            class Send
        >
        void serve_static_files(
            beast::string_view doc_root,
            boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>&& req,
            Send&& send
        )
        {
            m_logger->trace("serve_static_files()");

            // Get verb as string
            const boost::string_view& verb_str = boost::beast::http::to_string(req.method());

            // Make sure we can handle the method
            if (req.method() != boost::beast::http::verb::get &&
                req.method() != boost::beast::http::verb::head)
            {
                m_logger->debug("unknown HTTP-method: {}", verb_str.data());
                send(bad_request(req, "Unknown HTTP-method"));
                return;
            }

            // Build the path to the requested file
            std::string path = path_cat(doc_root, req.target());
            if (req.target().back() == '/')
                path.append("index.html");

            // Attempt to open the file
            beast::error_code ec;
            boost::beast::http::file_body::value_type body;
            body.open(path.c_str(), beast::file_mode::scan, ec);

            // Handle the case where the file doesn't exist
            if (ec == beast::errc::no_such_file_or_directory) {
                m_logger->debug("target not found: {}", std::string_view{ req.target().data(), req.target().size() });
                send(not_found(req, req.target()));
                return;
            }

            // Handle an unknown error
            if (ec) {
                m_logger->error("unhandled error: {}", ec.message());
                send(server_error(req, ec.message()));
                return;
            }

            // Cache the size since we need it after the move
            auto const size = body.size();

            // Respond to HEAD request
            if (req.method() == boost::beast::http::verb::head) {
                boost::beast::http::response<boost::beast::http::empty_body> res{boost::beast::http::status::ok, req.version()};
                res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(boost::beast::http::field::content_type, mime_type(path));
                res.content_length(size);
                res.keep_alive(req.keep_alive());

                return send(std::move(res));
            }

            // Respond to GET request
            boost::beast::http::response<boost::beast::http::file_body> res{
                    std::piecewise_construct,
                    std::make_tuple(std::move(body)),
                    std::make_tuple(boost::beast::http::status::ok, req.version())};
            res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(boost::beast::http::field::content_type, mime_type(path));
            res.content_length(size);
            res.keep_alive(req.keep_alive());

            return send(std::move(res));
        }

    private:
        std::shared_ptr<spdlog::logger> m_logger;
        std::vector<route_type> m_routes;

        // Append an HTTP rel-path to a local filesystem path.
        // The returned path is normalized for the platform.
        [[nodiscard]]
        static
        std::string
        path_cat(beast::string_view base, beast::string_view path);

        // Return a reasonable mime type based on the extension of a file.
        static beast::string_view mime_type(beast::string_view path);
    };
}
