#pragma once

#include "../../core/tcp/stream.hpp"

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

namespace malloy::server
{
    class router;
}

namespace malloy::server::http
{

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
         * @param router
         */
        connection_detector(
            std::shared_ptr<spdlog::logger> logger,
            boost::asio::ip::tcp::socket&& socket,
            std::shared_ptr<boost::asio::ssl::context> ctx,
            std::shared_ptr<const std::filesystem::path> doc_root,
            std::shared_ptr<malloy::server::router> router,
            std::string agent_string
        );

        /**
         * Launch the detector
         */
        void
        run();

    private:
        std::shared_ptr<spdlog::logger> m_logger;
        malloy::tcp::stream<> m_stream;
        std::shared_ptr<boost::asio::ssl::context> m_ctx;
        boost::beast::flat_buffer m_buffer;
        std::shared_ptr<const std::filesystem::path> m_doc_root;
        std::shared_ptr<malloy::server::router> m_router;
        std::string m_agent_string;

        void
        on_detect(boost::beast::error_code ec, bool result);
    };

}
