#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/error.hpp>

#include <filesystem>
#include <memory>
#include <string>

namespace boost::asio
{
    class io_context;
}

namespace boost::asio::ssl
{
    class context;
}

namespace spdlog
{
    class logger;
}

namespace malloy::server
{
    class router;

    /**
      * @brief Accepts incoming connections.
      */
    class listener :
        public std::enable_shared_from_this<listener>
    {
    public:
        /**
         * Constructor
         *
         * @param logger The logger instance to use.
         * @param ioc The I/O context to use.
         * @param tls_ctx The TLS context to use.
         * @param endpoint The enpoint to use.
         * @param router The router to use.
         * @param http_doc_root The path to the HTTP doc root.
         */
        listener(
            std::shared_ptr<spdlog::logger> logger,
            boost::asio::io_context& ioc,
            std::shared_ptr<boost::asio::ssl::context> tls_ctx,
            const boost::asio::ip::tcp::endpoint& endpoint,
            std::shared_ptr<malloy::server::router> router,
            std::shared_ptr<const std::filesystem::path> http_doc_root
        );

        /**
         * Copy constructor
         */
        listener(const listener& other) = delete;

        /**
         * Move constructor.
         */
        listener(listener&& other) noexcept = delete;

        /**
         * Destructor
         */
        virtual ~listener() = default;

        /**
         * Copy assignment operator.
         *
         * @param rhs The object to copy-assign from.
         * @return A reference to the assigned object.
         */
        listener& operator=(const listener& rhs) = delete;

        /**
         * Move assignment operator.
         *
         * @param rhs The object to move-assign from.
         * @return A reference to the assigned object.
         */
        listener& operator=(listener&& rhs) noexcept = delete;

        /**
         * Start accepting incoming connections
         */
        void run();

        /**
         * Get the router.
         *
         * @return The router.
         */
        [[nodiscard]]
        std::shared_ptr<malloy::server::router> router() const noexcept
        {
            return m_router;
        }

    private:
        std::shared_ptr<spdlog::logger> m_logger;
        boost::asio::io_context& m_io_ctx;
        std::shared_ptr<boost::asio::ssl::context> m_tls_ctx;
        boost::asio::ip::tcp::acceptor m_acceptor;
        std::shared_ptr<malloy::server::router> m_router;
        std::shared_ptr<const std::filesystem::path> m_doc_root;

        /**
         * Start accepting incoming requests.
         */
        void do_accept();

        /**
         * Asynchronous acceptor handler.
         *
         * @param ec The error code.
         * @param socket The socket.
         */
        void on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket);
    };

}
