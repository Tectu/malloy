#pragma once

#include <spdlog/logger.h>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_future.hpp>

#include <future>
#include <memory>
#include <thread>

namespace spdlog
{
    class logger;
}

namespace malloy
{
    namespace server {
        class controller;
    }
    namespace client {
        class controller;
    }
    namespace detail {

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
        };
        template<typename T>
        class controller_run_result {
        public:
            controller_run_result(const controller_config& cfg, T ctrl, std::unique_ptr<boost::asio::io_context> ioc) : m_ctrl{std::move(ctrl)}, m_io_ctx{std::move(ioc)}, m_workguard{m_io_ctx->get_executor()}
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
            ~controller_run_result() {
                // Stop the `io_context`. This will cause `run()`
                // to return immediately, eventually destroying the
                // `io_context` and all of the sockets in it.
                m_io_ctx->stop();

                // Tell the workguard that we no longer need it's service
                m_workguard.reset();


                for (auto& thread : m_io_threads) {
                    thread.join();
                };
            }

            /**
             * @brief Block until all queued async actions completed
             */
            void run() {
                m_workguard.reset();
                m_io_ctx->run();
            }

        private:
            using workguard_t = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

            T m_ctrl;
            std::unique_ptr<boost::asio::io_context> m_io_ctx;
            workguard_t m_workguard;
            std::vector<std::thread> m_io_threads;
        };
    }

    /**
     * Controller base class.
     *
     * Common controller base class from which both the client and server controller are derived from.
     */
    class controller
    {
    public:
        /**
         * The possible states the controller can be in.
         */
        enum class state
        {
            starting,
            running,
            stopping,
            stopped
        };
        using config = detail::controller_config;


        /**
         * Default constructor.
         */
        controller() = default;

        /**
         * Destructor.
         */
        virtual ~controller();

        /**
         * Stop the server.
         *
         * @return A future indicating when the stopping operation was completed.
         */
        [[nodiscard]]
        virtual
        std::future<void> stop();

    protected:

        [[nodiscard("init may fail")]]
        bool init(const config& cfg);

        /**
         * Get the I/O context.
         * @return A reference to the I/O context.
         */
        [[nodiscard]]
        boost::asio::io_context&
        io_ctx() const noexcept
        {
            return *m_io_ctx;
        }

        /**
         * Start the base controller.
         *
         * @param cfg The configuration.
         * @return Whether starting the base controller was successful.
         */
        [[nodiscard("start may fail")]]
        bool root_start(const config& cfg);

        /**
         * Remove the I/O context work guard.
         */
        void remove_workguard() const;

    private:
        using workguard_t = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

        bool m_init_done = false;
        std::unique_ptr<workguard_t> m_workguard;
        std::shared_ptr<boost::asio::io_context> m_io_ctx;
        std::vector<std::thread> m_io_threads;
        std::atomic<enum state> m_state = state::stopped;
    };

}
