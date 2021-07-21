#include "application.hpp"
#include "routing/router.hpp"

#include <spdlog/logger.h>

#include <stdexcept>

using namespace malloy::server;

application::application(
    std::shared_ptr<spdlog::logger> logger,
    std::string name
) :
    m_logger(std::move(logger)),
    m_name(std::move(name))
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

std::shared_ptr<application>
application::make_subapp(
    const std::string& name,
    std::string target_base
)
{
    // Sanity check
    if (name.empty()) {
        m_logger->warn("cannot create sub-app. name must not be empty.");
        return {};
    }

    // Create and setup app
    std::shared_ptr<application> app;
    try {
        // Create app
        app = std::make_shared<application>(
            m_logger->clone(m_logger->name() + " | " + name),
            name
        );

        // Add the sub-router
        if (!m_router->add_subrouter(std::move(target_base), app->router())) {
            m_logger->error("could not add router of created sub-app");
            return { };
        }
    }
    catch (const std::exception& e) {
        m_logger->error("could not create sub-application. exception: ", e.what());
        return { };
    }

    // House keeping
    m_subapps.emplace_back(app);

    m_logger->debug("added sub-application \"{}\"", name);

    return app;
}
