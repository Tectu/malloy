#include "router.hpp"

#include <stdexcept>

using namespace malloy::http::server;

router::router(std::shared_ptr<spdlog::logger> logger) :
        m_logger(std::move(logger))
{
    validate_logger();
}

void router::validate_logger()
{
    if (not m_logger)
        throw std::runtime_error("received invalid logger instance.");
}

void router::set_logger(std::shared_ptr<spdlog::logger> logger)
{
    m_logger = std::move(logger);

    validate_logger();
}

void router::add(const method_type method, const std::string_view target, std::function<response_type(const request_type&)>&& handler)
{
    validate_logger();

    // Log
    m_logger->debug("adding route: {}", target);

    // Check handler
    if (not handler) {
        m_logger->warn("route has invalid handler. ignoring.");
        return;
    }

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
    r.method = method;
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

std::shared_ptr<router> router::add_subrouter(std::string resource)
{
    validate_logger();

    // Log
    m_logger->debug("adding router: {}", resource);

    // Sanity check target
    {
        if (resource.empty()) {
            m_logger->error("invalid target \"{}\". not adding router.", resource);
            return { };
        }
    }

    // Create the sub-router
    auto sub_router = std::make_shared<router>(m_logger->clone("router " + resource));

    // Add router
    try {
        m_routers.try_emplace(std::move(resource), sub_router);
    }
    catch (const std::exception& e) {
        m_logger->critical("could not add router: {}", e.what());
        return { };
    }

    return sub_router;
}

void router::add_file_serving(std::string resource, std::filesystem::path storage_base_path)
{
    validate_logger();

    // Log
    m_logger->debug("adding file serving location: {} -> {}", resource, storage_base_path.string());

    // Add
    try {
        m_file_servings.try_emplace(std::move(resource), std::move(storage_base_path));
    }
    catch (const std::exception& e) {
        m_logger->critical("could not add file serving: {}", e.what());
        return;
    }
}

void router::add_redirect(const http::status status, std::string&& resource_old, std::string&& resource_new)
{
    validate_logger();

    // Log
    m_logger->debug("adding redirection: {}: {} -> {}", static_cast<int>(status), resource_old, resource_new);

    // Sanity check status
    if (static_cast<int>(status) < 300 or static_cast<int>(status) >= 400) {
        m_logger->error("invalid redirection status code. must be one of the 3xxx status codes. received {} instead.", static_cast<int>(status));
        return;
    }

    // Create record
    redirection_record record;
    record.status = status;
    record.resource_old = std::move(resource_old);
    record.resource_new = std::move(resource_new);

    // Add
    try {
        m_redirects.emplace_back(std::move(record));
    }
    catch (const std::exception& e) {
        m_logger->critical("could not add redirection record: {}", e.what());
        return;
    }
}
