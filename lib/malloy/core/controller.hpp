#pragma once

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>

#include <future>
#include <memory>
#include <thread>

namespace spdlog
{
    class logger;
}

namespace malloy
{

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

        /**
         * Controller configuration.
         */
        struct config
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
