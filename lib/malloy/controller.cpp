#include "controller.hpp"

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <spdlog/logger.h>

using namespace malloy;

controller::~controller()
{
    stop().wait();
}

bool controller::init(config cfg)
{
    // Don't initialize if not stopped
    if (m_state != state::stopped)
        return false;

    // Don't re-initialize
    if (m_init_done)
        return false;
    m_init_done = true;

    // Sanity check logger
    if (!cfg.logger)
        throw std::invalid_argument("no valid logger provided.");

    // Sanity check number of threads
    if (cfg.num_threads < 1) {
        cfg.logger->error("num_threads must be >= 1");
        return false;
    }

    // Grab the config
    m_cfg = std::move(cfg);

    // Create the I/O context
    m_io_ctx = std::make_shared<boost::asio::io_context>();

    
    return true;
}

bool controller::start()
{
    // Sanity check
    if (!m_io_ctx) {
        m_cfg.logger->critical("no I/O context present. Make sure that init() was called and succeeded.");
        return false;
    }

    // Create the I/O context threads
    m_io_threads.reserve(m_cfg.num_threads - 1);
    for (std::size_t i = 0; i < m_cfg.num_threads; i++) {
        m_io_threads.emplace_back(
            [this]
            {
                m_io_ctx->run();
            }
        );
    }

    // Log
    m_cfg.logger->debug("starting i/o context.");

    // Update state
    m_state = state::running;

    return true;
}

std::future<void> controller::stop()
{
    // Wait for all I/O context threads to finish...
    return std::async(
        [this]
        {
            // Check state
            if (m_state != state::running)
                return;

            // Update state
            m_state = state::stopping;

            // Stop the `io_context`. This will cause `run()`
            // to return immediately, eventually destroying the
            // `io_context` and all of the sockets in it.
            m_io_ctx->stop();


            m_cfg.logger->debug("waiting for I/O threads to stop...");

            for (auto& thread : m_io_threads)
                thread.join();

            m_state = state::stopped;

            m_cfg.logger->debug("all I/O threads stopped.");
        }
    );
}
