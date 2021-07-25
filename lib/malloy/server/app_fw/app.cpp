#include "app.hpp"
#include "page.hpp"
#include "../routing/router.hpp"

#include <spdlog/logger.h>

#include <stdexcept>

using namespace malloy::server;
using namespace malloy::server::app_fw;

app::app(
    std::shared_ptr<spdlog::logger> logger,
    std::string name,
    environment env
) :
    m_logger(std::move(logger)),
    m_name(std::move(name)),
    m_env(std::move(env))
{
    // Sanity check logger
    if (!m_logger)
        throw std::invalid_argument("no valid logger provided.");

    // Sanity check name
    if (m_name.empty())
        throw std::invalid_argument("application name must not be empty.");

    // Create router
    m_router = std::make_shared<malloy::server::router>();
}

bool
app::add_page(std::string&& target, std::shared_ptr<page> page)
{
    // Sanity check
    if (!page)
        return false;

    // Add endpoint
    return m_router->add(
        malloy::http::method::get,
        std::move(target),
        [page = std::move(page)]([[maybe_unused]] const auto& req) {
            return page->render();
        }
    );
}

bool
app::add_subapp(std::shared_ptr<app> app)
{
    // Sanity check
    if (!app)
        return false;

    // Sanity check
    if (app->name().empty()) {
        m_logger->warn("cannot create sub-app. name must not be empty.");
        return false;
    }

    // Add the sub-router
    const std::string target_base = "/" + std::string{ app->name() };
    if (!m_router->add_subrouter(target_base, app->router())) {
        m_logger->error("could not add router of sub-app");
        return false;
    }

    // Log
    m_logger->debug("added sub-application \"{}\"", app->name());

    // House keeping
    m_subapps.emplace_back(std::move(app));

    return true;
}
