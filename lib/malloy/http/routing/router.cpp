#include "router.hpp"
#include "route_basic.hpp"
#include "route_files.hpp"
#include "route_redirect.hpp"

#include <stdexcept>

using namespace malloy::http::server;

router::router(std::shared_ptr<spdlog::logger> logger) :
    m_logger(std::move(logger))
{
}

void router::set_logger(std::shared_ptr<spdlog::logger> logger)
{
    m_logger = std::move(logger);
}

bool router::add_route(std::shared_ptr<route<request_type, response_type>>&& r)
{
    try {
        m_routes.emplace_back(std::move(r));
    }
    catch (const std::exception& e) {
        return log_or_throw(e, spdlog::level::critical, "could not add route: {}", e.what());
    }

    return true;
}

bool router::add(const method_type method, const std::string_view target, std::function<response_type(const request_type&)>&& handler)
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

    // Build route
    auto route = std::make_shared<route_basic<request_type, response_type>>();
    route->resource_base = std::move(regex);
    route->method = method;
    route->handler = std::move(handler);

    // Add route
    return add_route(std::move(route));
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

bool router::add_file_serving(std::string resource, std::filesystem::path storage_base_path)
{
    // Log
    if (m_logger)
        m_logger->debug("adding file serving location: {} -> {}", resource, storage_base_path.string());

    // Create route
    auto route = std::make_shared<route_files<request_type, response_type>>();
    route->resource_base = resource;
    route->base_path = std::move(storage_base_path);

    // Add
    return add_route(std::move(route));
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

    // Create route
    auto route = std::make_shared<route_redirect<request_type, response_type>>();
    route->resource_old = std::move(resource_old);
    route->resource_new = std::move(resource_new);
    route->status = status;

    // Add
    return add_route(std::move(route));
}
