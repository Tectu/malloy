#pragma once

#include <memory>
#include <string>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/error.hpp>

using tcp = boost::asio::ip::tcp;

namespace boost::asio
{
    class io_context;
}

namespace malloy::server::http
{
    class router;
}

namespace spdlog
{
    class logger;
}

namespace malloy::server
{

    /**
      * @brief Accepts incoming connections and launches the sessions
      */
    class listener :
        public std::enable_shared_from_this<listener>
    {
    public:
        // Construction
        listener(
                std::shared_ptr<spdlog::logger> logger,
                boost::asio::io_context& ioc,
                const boost::asio::ip::tcp::endpoint& endpoint,
                std::shared_ptr<http::router> router,
                std::shared_ptr<const std::string> http_doc_root
        );
        listener(const listener& other) = delete;
        listener(listener&& other) noexcept = delete;
        virtual ~listener() = default;

        // Operators
        listener& operator=(const listener& rhs) = delete;
        listener& operator=(listener&& rhs) noexcept = delete;

        // Start accepting incoming connections
        void run();

    private:
        std::shared_ptr<spdlog::logger> m_logger;
        boost::asio::io_context& m_io_ctx;
        tcp::acceptor m_acceptor;
        std::shared_ptr<http::router> m_router;
        std::shared_ptr<const std::string> m_doc_root;

        // Sync
        void do_accept();

        // Async
        void on_accept(boost::beast::error_code ec, tcp::socket socket);
    };

}
