#pragma once

#include "connection_t.hpp"
#include "../websocket/connection.hpp"
#include "../../core/http/request.hpp"
#include "../../core/http/generator.hpp"

#include <boost/asio/dispatch.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/detail/type_traits.hpp>
#include <spdlog/logger.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <concepts>
#include <vector>

namespace spdlog
{
    class logger;
}

namespace malloy::server::http
{

    /**
     * An HTTP server connection.
     *
     * @note This uses CRTP to allow using the same code for different connection types (eg. plain or TLS).
     *
     * @sa connection_plain
     * @sa connection_tls
     */
    template<class Derived>
    class connection
    {
    public:
        class request_generator :
            public std::enable_shared_from_this<request_generator>
        {
            friend class connection;

        public:
            using h_parser_t = std::unique_ptr<boost::beast::http::request_parser<boost::beast::http::empty_body>>;
            using header_t = boost::beast::http::request_header<>;

            [[nodiscard]]
            header_t&
            header() { return m_header; }

            [[nodiscard]]
            const header_t&
            header() const { return m_header; }

            template<typename Body, std::invocable<malloy::http::request<Body>&&> Callback, typename SetupCb>
            auto body(Callback&& done, SetupCb&& setup)
            {
                using namespace boost::beast::http;
                using body_t = std::decay_t<Body>;
                auto parser = std::make_shared<boost::beast::http::request_parser<body_t>>(std::move(*m_parser));
                parser->get().base() = m_header;
                std::invoke(setup, parser->get().body());

                boost::beast::http::async_read(
                    m_parent->derived().m_stream, m_buffer, *parser,
                    [_ = m_parent,
                     done = std::forward<Callback>(done),
                     p = parser, this_ = this->shared_from_this(), this](const auto& ec, auto) {
                        if (ec && m_parent->m_logger) {    // TODO: see #40
                            m_parent->m_logger->error("failed to read http request body: '{}'", ec.message());
                            return;
                        }
                        done(malloy::http::request<Body>{p->release()});
                    });
            }

            template<typename Body, std::invocable<malloy::http::request<Body>&&> Callback>
            auto body(Callback&& done)
            {
                return body<Body>(std::forward<Callback>(done), [](auto) {});
            }

        private:
            request_generator(h_parser_t hparser, header_t header, std::shared_ptr<connection> parent, boost::beast::flat_buffer buff) :
                m_buffer{ std::move(buff) },
                m_parser{ std::move(hparser) },
                m_header{ std::move(header) },
                m_parent{ std::move(parent) }
            {
                assert(m_parent); // ToDo: Should this be BOOST_ASSERT?
            }

            boost::beast::flat_buffer m_buffer;
            h_parser_t m_parser;
            header_t m_header;
            std::shared_ptr<connection> m_parent;
        };
        
        class handler
        {
        public:
            using request = malloy::http::request<>;
            using conn_t = const connection_t&;
            using path = std::filesystem::path;
            using req_t = std::shared_ptr<request_generator>;

            virtual void websocket(const path& root, const req_t& req, const std::shared_ptr<malloy::server::websocket::connection>&) = 0;
            virtual void http(const path& root, const req_t& req, conn_t) = 0;

            virtual ~handler() = default;
        };

        /**
         * Session configuration structure.
         */
        struct config
        {
            std::uint64_t request_body_limit = 100'000'000;    ///< The maximum allowed body request size in bytes.
            std::string agent_string;                          ///< Agent string to use, set by the controller
        };

        /**
         * The connection configuration.
         */
        struct config cfg;

        /**
         * Constructor
         *
         * @param logger
         * @param buffer
         * @param router
         * @param http_doc_root
         */
        connection(
            std::shared_ptr<spdlog::logger> logger,
            boost::beast::flat_buffer buffer,
            std::shared_ptr<handler> router,
            std::shared_ptr<const std::filesystem::path> http_doc_root
        ) :
            m_buffer(std::move(buffer)),
            m_logger(std::move(logger)),
            m_doc_root(std::move(http_doc_root)),
            m_router(std::move(router))
        {
            // Sanity check logger
            if (!m_logger)
                throw std::runtime_error("did not receive a valid logger instance.");

            // Sanity check router
            if (!m_router)
                throw std::runtime_error("did not receive a valid router instance.");
        }

        template<bool isRequest, class Body, class Fields>
        void
        do_write(boost::beast::http::message<isRequest, Body, Fields>&& msg)
        {
            // The lifetime of the message has to extend
            // for the duration of the async operation so
            // we use a shared_ptr to manage it.
            auto sp = std::make_shared<boost::beast::http::message<isRequest, Body, Fields>>(std::move(msg));

            // Store a type-erased version of the shared
            // pointer in the class to keep it alive.
            m_response = sp;

            // Write the response
            boost::beast::http::async_write(
                derived().m_stream,
                *sp,
                boost::beast::bind_front_handler(
                    &connection::on_write,
                    derived().shared_from_this(),
                    sp->need_eof()
                )
            );
        }

        void
        do_read()
        {
            m_logger->trace("do_read()");

            // Construct a new parser for each message
            m_parser = std::make_unique<std::decay_t<decltype(*m_parser)>>();

            // Apply a reasonable limit to the allowed size
            // of the body in bytes to prevent abuse.
            m_parser->body_limit(cfg.request_body_limit);

            // Set the timeout.
            boost::beast::get_lowest_layer(derived().stream()).expires_after(std::chrono::seconds(30));

            // Read a request using the parser-oriented interface
            boost::beast::http::async_read_header(
                derived().m_stream,
                m_buffer,
                *m_parser,
                boost::beast::bind_front_handler(
                    &connection::on_read,
                    derived().shared_from_this()
                )
            );
        }

    protected:
        boost::beast::flat_buffer m_buffer;

        void
        report_err(malloy::error_code ec, std::string_view context)
        {
            m_logger->error("{}: {} (code: {})", context, ec.message(), ec.value());
        }

    private:
        friend class request_generator;

        std::shared_ptr<spdlog::logger> m_logger;
        std::shared_ptr<const std::filesystem::path> m_doc_root;
        std::shared_ptr<handler> m_router;
        std::shared_ptr<void> m_response;

        // Pointer to allow handoff to generator since it cannot be copied or moved
        typename request_generator::h_parser_t m_parser;

        /**
         * Cast to derived class type.
         *
         * @return Reference to the derived class type.
         */
        [[nodiscard]]
        Derived& derived()
        {
            return static_cast<Derived&>(*this);
        }

        void
        on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
        {
            m_logger->trace("on_read(): bytes read: {}", bytes_transferred);

            // This means the connection was closed
            if (ec == boost::beast::http::error::end_of_stream || ec == boost::beast::error::timeout)
                return do_close();

            // Check for errors
            if (ec) {
                m_logger->error("on_read(): {}", ec.message());
                return;
            }

            auto header = m_parser->get().base();
            // Parse the request into something more useful from hereon
            auto gen = std::shared_ptr<request_generator>{new request_generator{  std::move(m_parser), std::move(header), derived().shared_from_this(), std::move(m_buffer) }}; // Private ctor

            // Check if this is a WS request
            if (boost::beast::websocket::is_upgrade(gen->header())) {
                m_logger->debug("upgrading HTTP connection to WS connection.");

                // Create a websocket connection, transferring ownership
                // of both the socket and the HTTP request.
                boost::beast::get_lowest_layer(derived().stream()).expires_never();

                auto ws_connection = server::websocket::connection::make(
                    m_logger->clone("websocket_connection"),
                    malloy::websocket::stream{derived().release_stream()}, cfg.agent_string);
                m_router->websocket(*m_doc_root, gen, ws_connection);


            }

            // This is an HTTP request
            else {
                // Hand over to router
                m_router->http(*m_doc_root, std::move(gen), derived().shared_from_this());
            }
        }

        void
        on_write(bool close, boost::beast::error_code ec, std::size_t bytes_transferred)
        {
            m_logger->trace("on_write(): bytes written: {}", bytes_transferred);

            // Check for errors
            if (ec) {
                m_logger->error("on_write(): {}", ec.message());
                return;
            }

            if (close) {
                // This means we should close the connection, usually because
                // the response indicated the "Connection: close" semantic.
                return do_close();
            }

            // We're done with the response so delete it
            m_response = { };

            // Read another request
            do_read();
        }

        /**
         * Close the connection.
         */
        void
        do_close()
        {
            m_logger->trace("do_close()");

            // Let the derived class handle this
            derived().do_close();

            // At this point the connection is closed gracefully
            m_logger->trace("closed HTTP connection gracefully.");
        }
    };

}
