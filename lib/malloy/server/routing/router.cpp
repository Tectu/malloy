#include "router.hpp"
#include "endpoint_http_files.hpp"
#include "endpoint_http_redirect.hpp"

#include <algorithm>
#include <stdexcept>

using namespace malloy::server;

router::router(std::shared_ptr<spdlog::logger> logger, std::string server_str) :
    m_logger(std::move(logger)), m_server_str{std::move(server_str)}
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
    } catch (const std::exception& e) {
        return log_or_throw(e, spdlog::level::critical, "could not add HTTP endpoint: {}", e.what());
    } catch (...) {
        return log_or_throw(std::runtime_error("unknown exception"), spdlog::level::critical, "could not add HTTP endpoint. unknown exception.");
    }

    return true;
}

bool router::add_websocket_endpoint(std::shared_ptr<endpoint_websocket>&& ep)
{
    try {
        m_endpoints_websocket.emplace_back(std::move(ep));
    } catch (const std::exception& e) {
        return log_or_throw(e, spdlog::level::critical, "could not add WebSocket endpoint: {}", e.what());
    } catch (...) {
        return log_or_throw(std::runtime_error("unknown exception"), spdlog::level::critical, "could not add WebSocket endpoint. unknown exception.");
    }

    return true;
}

bool router::add_subrouter(std::string resource, std::shared_ptr<router> sub_router)
{
    // Log
    if (m_logger)
        m_logger->trace("adding router: {}", resource);

    // Sanity check target
    if (resource.empty()) {
        if (m_logger)
            m_logger->error("invalid target \"{}\". not adding router.", resource);
        return false;
    }

    // Sanity check router
    if (!sub_router) {
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
    } catch (const std::exception& e) {
        return log_or_throw(e, spdlog::level::critical, "could not add router: {}", e.what());
    }

    return true;
}

bool router::add_preflight(const std::string_view target, http::preflight_config cfg)
{
    return add(
        malloy::http::method::options,
        target,
        [cfg = std::move(cfg), this](const auto& req) {
            // Look for matching endpoints
            std::vector<std::shared_ptr<const endpoint_http>> endpoints;
            std::copy_if(
                std::cbegin(m_endpoints_http),
                std::cend(m_endpoints_http),
                std::back_inserter(endpoints),
                [req](const auto& ep){
                    // Only support this for endpoint_http_regex (for now?)
                    const auto& matcher = std::dynamic_pointer_cast<resource_matcher>(ep);
                    if (!matcher)
                        return false;

                    return matcher->matches_resource(req);
                }
            );

            // Assemble list of methods
            std::vector<malloy::http::method> methods;
            methods.resize(endpoints.size());
            std::transform(
                std::cbegin(endpoints),
                std::cend(endpoints),
                std::begin(methods),
                [](const auto& ep) {
                    return ep->method;
                }
            );

            // Generate response
            malloy::http::response resp{ malloy::http::status::ok };
            cfg.setup_response(resp, methods);

            return resp;
        }
    );
}

bool router::add_file_serving(std::string resource, std::filesystem::path storage_base_path)
{
    // Log
    if (m_logger)
        m_logger->trace("adding file serving location: {} -> {}", resource, storage_base_path.string());

    // Create endpoint
    auto ep = std::make_shared<endpoint_http_files>();

    ep->resource_base = resource;
    ep->base_path = std::move(storage_base_path);
    ep->writer = make_endpt_writer_callback();

    // Add
    return add_http_endpoint(std::move(ep));
}

bool router::add_redirect(const malloy::http::status status, std::string&& resource_old, std::string&& resource_new)
{
    // Log
    if (m_logger)
        m_logger->trace("adding redirection: {}: {} -> {}", static_cast<int>(status), resource_old, resource_new);

    // Sanity check status
    if (static_cast<int>(status) < 300 || static_cast<int>(status) >= 400) {
        if (m_logger)
            m_logger->error("invalid redirection status code. must be one of the 3xxx status codes. received {} instead.", static_cast<int>(status));
        return false;
    }

    // Sanity check old resource
    if (!resource_old.starts_with("/"))
        return false;

    // Sanity check new resource
    if (!resource_new.starts_with("/"))
        return false;

    // Create endpoint
    auto ep = std::make_shared<endpoint_http_redirect>();
    ep->resource_old = std::move(resource_old);
    ep->resource_new = std::move(resource_new);
    ep->status = status;

    // Add
    return add_http_endpoint(std::move(ep));
}

bool router::add_websocket(std::string&& resource, typename websocket::connection::handler_t&& handler)
{
    // Log
    if (m_logger)
        m_logger->trace("adding websocket endpoint at {}", resource);

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
