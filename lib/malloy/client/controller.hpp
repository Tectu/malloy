#pragma once

#include "../websocket/types.hpp"

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <spdlog/logger.h>

#include <future>
#include <memory>
#include <thread>

namespace boost::asio
{
    class io_context;
}

namespace malloy::client
{
    namespace websocket
    {
        class connection_plain;
    }

    class controller
    {
    public:
        struct config
        {
            /**
             * The number of worked threads for the I/O context to use.
             */
            std::size_t num_threads        = 1;

            /**
             * The logger instance to use.
             * A logger will be automatically created if none was provided.
             */
            std::shared_ptr<spdlog::logger> logger;
        };

        controller() = default;
        virtual ~controller();

        [[nodiscard("init may fail")]]
        bool init(config cfg);

        [[nodiscard("start may fail")]]
        bool start();

        std::future<void> stop();

        template<class Connection>
        [[nodiscard]]
        std::shared_ptr<Connection>
        make_websocket_connection(const std::string& host, std::uint16_t port, const std::string& endpoint, malloy::websocket::handler_t&& handler)
        {
            // Sanity check
            if (!handler)
                return { };

            // Create connection
            auto conn = std::make_shared<Connection>(m_cfg.logger->clone("connection"), *m_io_ctx, std::move(handler));

            // Launch the connection
            conn->connect(host, std::to_string(port), endpoint);

            return conn;
        }

        void test_tls();

    private:
        using workguard_t = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

        config m_cfg;
        std::unique_ptr<workguard_t> m_workguard;
        std::shared_ptr<boost::asio::io_context> m_io_ctx;
        std::vector<std::thread> m_io_threads;
    };

}
