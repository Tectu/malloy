#pragma once

#include "websocket/types.hpp"

#include <boost/asio/io_context.hpp>

#include <memory>
#include <filesystem>
#include <future>
#include <string>
#include <thread>
#include <vector>

namespace spdlog
{
    class logger;
}

namespace malloy::http::server
{
    class router;
}

namespace malloy::server
{

    class listener;

    class controller
    {
    public:
        struct config
        {
            std::string interface          = "127.0.0.1";
            std::uint16_t port             = 8080;
            std::filesystem::path doc_root = ".";
            std::size_t num_threads        = 1;
            std::shared_ptr<spdlog::logger> logger;

            config() = default;
            config(const config& other) = default;
            config(config&& other) noexcept = default;
            virtual ~config() = default;

            config& operator=(const config& cfg) = default;
            config& operator=(config&& cfg) noexcept = default;
        };

        controller() = default;
        controller(const controller& other) = delete;
        controller(controller&& other) noexcept = delete;
        virtual ~controller();

        controller& operator=(const controller& rhs) = delete;
        controller& operator=(controller&& rhs) noexcept = delete;

        bool init(config cfg);
        void enable_termination_signals();
        bool start();
        std::future<void> stop();
        [[nodiscard]] std::shared_ptr<malloy::http::server::router> router() const;
        void set_websocket_handler(malloy::websocket::handler_type handler);

    private:
        bool m_init_done = false;
        config m_cfg;
        std::shared_ptr<listener> m_listener;
        std::vector<std::thread> m_threads;
        boost::asio::io_context m_io_ctx;
    };

}
