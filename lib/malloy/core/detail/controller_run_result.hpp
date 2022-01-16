#pragma once

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_future.hpp>
#include <spdlog/logger.h>

#include <memory>
#include <thread>
#include <stdexcept>

namespace malloy::detail
{

    /**
     * Controller configuration.
    */
    struct controller_config
    {
        /**
             * The number of worked threads for the I/O context to use.
             */
        std::size_t num_threads = 1;

        /**
             * The logger instance to use.
             */
        std::shared_ptr<spdlog::logger> logger;

        void validate()
        {
            if (!logger)
                throw std::logic_error{"invalid config: logger is null"};
            else if (num_threads == 0)
                throw std::logic_error{"invalid config: cannot have 0 threads"};
        };
    };

    template<typename T>
        requires std::movable<T>
    class controller_run_result
    {
    public:
        /**
         * Constructor.
         *
         * @param cfg The controller configuration.
         * @param ctrl The controller.
         * @param ioc The I/O context.
         */
        controller_run_result(const controller_config& cfg, T ctrl, std::unique_ptr<boost::asio::io_context> ioc) :
            m_io_ctx{std::move(ioc)},
            m_workguard{m_io_ctx->get_executor()},
            m_ctrl{std::move(ctrl)}
        {
            // Create the I/O context threads
            m_io_threads.reserve(cfg.num_threads - 1);
            for (std::size_t i = 0; i < cfg.num_threads; i++) {
                m_io_threads.emplace_back(
                    [this] {
                        m_io_ctx->run();
                    });
            }

            // Log
            cfg.logger->debug("starting i/o context.");
        }

        /**
         * Move constructor.
         */
        controller_run_result(controller_run_result&&) noexcept = default;

        /**
         * Move-assignment operator.
         */
        controller_run_result& operator=(controller_run_result&&) noexcept = default;

        controller_run_result(const controller_run_result&) = delete;
        controller_run_result& operator=(const controller_run_result&) = delete;

        /**
         * Destructor.
         */
        ~controller_run_result()
        {
            // Stop the `io_context`. This will cause `run()`
            // to return immediately, eventually destroying the
            // `io_context` and all of the sockets in it.
            if (m_io_ctx)
                m_io_ctx->stop();

            // Tell the workguard that we no longer need it's service
            m_workguard.reset();

            // Join threads
            for (auto& thread : m_io_threads)
                thread.join();
        }

        /**
         * @brief Block until all queued async actions completed
         */
        void run()
        {
            m_workguard.reset();
            m_io_ctx->run();
        }

    private:
        using workguard_t = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

        std::unique_ptr<boost::asio::io_context> m_io_ctx;
        workguard_t m_workguard;
        std::vector<std::thread> m_io_threads;
        T m_ctrl;    // This order matters, the T may destructor need access to something related to the io context
    };

}    // namespace malloy::detail
