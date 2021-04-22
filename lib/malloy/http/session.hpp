#pragma once

#include "../websocket/types.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace spdlog
{
    class logger;
}

namespace malloy::http::server
{
    class router;

    /**
     * An HTTP session.
     *
     * @brief Handles an HTTP server connection.
     */
    class session :
        public std::enable_shared_from_this<session>
    {
        /**
         * This queue is used for HTTP pipelining.
         */
        class queue
        {
            enum
            {
                // Maximum number of responses we will queue
                limit = 8
            };

            // The type-erased, saved work item
            struct work
            {
                virtual ~work() = default;
                virtual void operator()() = 0;
            };

            session& m_self;
            std::vector<std::unique_ptr<work>> m_items;

        public:
            explicit queue(session& self) :
                m_self(self)
            {
                static_assert(limit > 0, "queue limit must be positive");
                m_items.reserve(limit);
            }

            // Returns `true` if we have reached the queue limit
            [[nodiscard]]
            bool is_full() const
            {
                return m_items.size() >= limit;
            }

            // Called when a message finishes sending
            // Returns `true` if the caller should initiate a read
            bool on_write()
            {
                BOOST_ASSERT(! m_items.empty());
                auto const was_full = is_full();
                m_items.erase(m_items.begin());
                if(! m_items.empty())
                    (*m_items.front())();
                return was_full;
            }

            // Called by the HTTP handler to send a response.
            template<bool isRequest, class Body, class Fields>
            void operator()(boost::beast::http::message<isRequest, Body, Fields>&& msg)
            {
                // This holds a work item
                struct work_impl :
                    work
                {
                    session& m_self;
                    boost::beast::http::message<isRequest, Body, Fields> m_msg;

                    work_impl(session& self, boost::beast::http::message<isRequest, Body, Fields>&& msg) :
                        m_self(self),
                        m_msg(std::move(msg))
                    {
                    }

                    void operator()()
                    {
                        boost::beast::http::async_write(
                            m_self.m_stream,
                            m_msg,
                            boost::beast::bind_front_handler(
                                &session::on_write,
                                m_self.shared_from_this(),
                                m_msg.need_eof()));
                    }
                };

                // Allocate and store the work
                m_items.push_back(std::make_unique<work_impl>(m_self, std::move(msg)));

                // If there was no previous work, start this one
                if (m_items.size() == 1)
                    (*m_items.front())();
            }
        };

    public:
        /**
         * Session configuration structure.
         */
        struct config
        {
            std::uint64_t request_body_limit = 10 * 10e6;   ///< The maximum allowed body request size in bytes.
        };

        /**
         * The session configuration.
         */
        struct config cfg;

        /**
         * Constructor
         *
         * @param logger
         * @param socket
         * @param router
         * @param http_doc_root
         */
        session(
            std::shared_ptr<spdlog::logger> logger,
            boost::asio::ip::tcp::socket&& socket,
            std::shared_ptr<class router> router,
            std::shared_ptr<const std::filesystem::path> http_doc_root,
            malloy::websocket::handler_type websocket_handler
        );

        /**
         * Start the session.
         */
        void run();

    private:
        std::shared_ptr<spdlog::logger> m_logger;
        boost::beast::tcp_stream m_stream;
        boost::beast::flat_buffer m_buffer;
        std::shared_ptr<const std::filesystem::path> m_doc_root;
        std::shared_ptr<router> m_router;
        malloy::websocket::handler_type m_websocket_handler;
        queue m_queue;

        // The parser is stored in an optional container so we can
        // construct it from scratch it at the beginning of each new message.
        boost::optional<boost::beast::http::request_parser<boost::beast::http::string_body>> m_parser;

        void do_read();
        void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);
        void on_write(bool close, boost::beast::error_code ec, std::size_t bytes_transferred);
        void do_close();
    };

}
