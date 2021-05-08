#pragma once

#include "../../websocket/types.hpp"

#include <boost/beast/core.hpp>

#include <filesystem>

namespace boost::asio::ssl
{
    class context;
}

namespace spdlog
{
    class logger;
}

namespace malloy::http::server
{
    class router;

    /**
     * This class is used to detect plain or TLS connections.
     * This is done by looking for a TLS handshake.
     */
    class connection_detector :
        public std::enable_shared_from_this<connection_detector>
    {
    public:
        /**
         * Constructor.
         *
         * @param logger
         * @param socket
         * @param ctx
         * @param doc_root
         */
        connection_detector(
            std::shared_ptr<spdlog::logger> logger,
            boost::asio::ip::tcp::socket&& socket,
            boost::asio::ssl::context& ctx,
            std::shared_ptr<const std::filesystem::path> doc_root,
            std::shared_ptr<http::server::router> router,
            malloy::websocket::handler_type websocket_handler
        );

        /**
         * Launch the detector
         */
        void run();

    private:
        std::shared_ptr<spdlog::logger> m_logger;
        boost::beast::tcp_stream m_stream;
        boost::asio::ssl::context& m_ctx;
        boost::beast::flat_buffer m_buffer;
        std::shared_ptr<const std::filesystem::path> m_doc_root;
        std::shared_ptr<http::server::router> m_router;
        malloy::websocket::handler_type m_websocket_handler;

        void on_detect(boost::beast::error_code ec, bool result);
    };

}
