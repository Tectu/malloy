#include "router.hpp"
#include "endpoint_http_files.hpp"
#include "endpoint_http_redirect.hpp"
#include "endpoint_http_regex.hpp"
#include "endpoint_websocket.hpp"

#include <stdexcept>

using namespace malloy::server;

router::router(std::shared_ptr<spdlog::logger> logger) :
    m_logger(std::move(logger))
{
}

void router::set_logger(std::shared_ptr<spdlog::logger> logger)
{
    m_logger = std::move(logger);
}

bool router::add_http_endpoint(std::shared_ptr<endpoint_http>&& ep)
{
    try {
        m_endpoints_http.emplace_back(std::move(ep));
    }
    catch (const std::exception& e) {
        return log_or_throw(e, spdlog::level::critical, "could not add HTTP endpoint: {}", e.what());
    }
    catch (...) {
        return log_or_throw(std::runtime_error("unknown exception"), spdlog::level::critical, "could not add HTTP endpoint. unknown exception.");
    }

    return true;
}

bool router::add_websocket_endpoint(std::shared_ptr<endpoint_websocket>&& ep)
{
    try {
        m_endpoints_websocket.emplace_back(std::move(ep));
    }
    catch (const std::exception& e) {
        return log_or_throw(e, spdlog::level::critical, "could not add WebSocket endpoint: {}", e.what());
    }
    catch (...) {
        return log_or_throw(std::runtime_error("unknown exception"), spdlog::level::critical, "could not add WebSocket endpoint. unknown exception.");
    }

    return true;
}

bool router::add_subrouter(std::string resource, std::shared_ptr<router> sub_router)
{
    // Log
    if (m_logger)
        m_logger->debug("adding router: {}", resource);

    // Sanity check target
    if (resource.empty()) {
        if (m_logger)
            m_logger->error("invalid target \"{}\". not adding router.", resource);
        return false;
    }

    // Sanity check router
    if (not sub_router) {
        if (m_logger)
            m_logger->error("invalid sub-router supplied.");
        return false;
    }

    // Set the sub-router's logger
    if (m_logger)
        sub_router->set_logger(m_logger->clone(m_logger->name() + " | " + resource));

    // Add router
    try {
        m_sub_routers.try_emplace(std::move(resource), std::move(sub_router));
    }
    catch (const std::exception& e) {
        return log_or_throw(e, spdlog::level::critical, "could not add router: {}", e.what());
    }

    return true;
}

bool router::add(const method_type method, const std::string_view target, endpoint_http_regex::handler_t&& handler)
{
    // Log
    if (m_logger)
        m_logger->debug("adding route: {}", target);

    // Check handler
    if (not handler) {
        if (m_logger)
            m_logger->warn("route has invalid handler. ignoring.");
        return false;
    }

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

    // Build endpoint
    auto ep = std::make_shared<endpoint_http_regex>();
    ep->resource_base = std::move(regex);
    ep->method = method;
    ep->handler = std::move(handler);

    // Add route
    return add_http_endpoint(std::move(ep));
}

bool router::add_file_serving(std::string resource, std::filesystem::path storage_base_path)
{
    // Log
    if (m_logger)
        m_logger->debug("adding file serving location: {} -> {}", resource, storage_base_path.string());

    // Create endpoint
    auto ep = std::make_shared<endpoint_http_files>();
    ep->resource_base = resource;
    ep->base_path = std::move(storage_base_path);

    // Add
    return add_http_endpoint(std::move(ep));
}

bool router::add_redirect(const http::status status, std::string&& resource_old, std::string&& resource_new)
{
    // Log
    if (m_logger)
        m_logger->debug("adding redirection: {}: {} -> {}", static_cast<int>(status), resource_old, resource_new);

    // Sanity check status
    if (static_cast<int>(status) < 300 or static_cast<int>(status) >= 400) {
        if (m_logger)
            m_logger->error("invalid redirection status code. must be one of the 3xxx status codes. received {} instead.", static_cast<int>(status));
        return false;
    }

    // Create endpoint
    auto ep = std::make_shared<endpoint_http_redirect>();
    ep->resource_old = std::move(resource_old);
    ep->resource_new = std::move(resource_new);
    ep->status = status;

    // Add
    return add_http_endpoint(std::move(ep));
}

bool router::add_websocket(std::string resource, malloy::websocket::handler_t handler)
{
    // Log
    if (m_logger)
        m_logger->debug("adding websocket endpoint at {}", resource);

    // Check handler
    if (not handler) {
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

router::response_type router::generate_preflight_response(const request_type& req) const
{
    // Create list of methods
    std::vector<std::string> method_strings;
    for (const auto& route : m_endpoints_http) {
        // Only support this for regex routes (for now?)
        const auto& basic_route = std::dynamic_pointer_cast<endpoint_http_regex>(route);
        if (not basic_route)
            continue;

        // Check match
        if (not basic_route->matches_resource(req))
            continue;

        // Add method string
        method_strings.emplace_back(boost::beast::http::to_string(basic_route->method));
    }

    // Create a string representing all supported methods
    std::string methods_string;
    for (const auto &str : method_strings) {
        methods_string += str;
        if (&str not_eq
            &method_strings.back())
            methods_string += ", ";
    }

    http::response resp{ http::status::ok };
    resp.set(boost::beast::http::field::content_type, "text/html");
    resp.base().set("Access-Control-Allow-Origin", "http://127.0.0.1:8080");
    resp.base().set("Access-Control-Allow-Methods", methods_string);
    resp.base().set("Access-Control-Allow-Headers", "Content-Type");
    resp.base().set("Access-Control-Max-Age", "60");

    return resp;
}