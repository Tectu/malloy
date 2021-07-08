#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/beast/core/error.hpp>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

#include "malloy/error.hpp"
#include "malloy/http/request.hpp"
#include "malloy/type_traits.hpp"
#include "malloy/utils.hpp"
#include "malloy/websocket/stream.hpp"
#include "malloy/websocket/types.hpp"
#include "malloy/error.hpp"

#include <concepts>
#include <functional>
#include <memory>

namespace malloy::websocket
{
    namespace detail
    {
        constexpr std::string_view beast_version = BOOST_BEAST_VERSION_STRING;
        template<bool isClient>
        constexpr auto ws_agent_string() -> std::string_view
        {
            if (isClient) {
                return {BOOST_BEAST_VERSION_STRING " websocket-client-async"};
            } else {
                return {BOOST_BEAST_VERSION_STRING " malloy"};
            }
        }
    }    // namespace detail

    /**
     * @class connection
     * @tparam isClient: Whether it is the client end of a websocket connection
     * @brief Represents a connection via the WebSocket protocol
     * @details Provides basic management of a websocket connection. Will close the
     * connection on destruction. The interface is entirely asynchronous and uses
     * callback functions
     */
    template<bool isClient>
    class connection :
        public std::enable_shared_from_this<connection<isClient>>
    {
    public:
        using handler_t = std::function<void(const malloy::http::request<>&, const std::shared_ptr<connection>&)>;
        enum class state
        {
            handshaking,
            active,
            closing,
            closed
        };

        /**
         * The agent string.
         */
        constexpr static std::string_view agent_string = detail::ws_agent_string<isClient>();

        /**
         * See stream::set_binary(bool)
         */
        void set_binary(const bool enabled) { m_ws.set_binary(enabled); }

        /**
         * See stream::binary()
         */
        [[nodiscard]]
        bool binary() { return m_ws.binary(); }

        /**
         * @brief Construct a new connection object
         * @param logger Logger to use. Must not be `nullptr`
         * @param ws Stream to use. May be unopened/connected but in that case
         * `connect` must be called before this connection can be used
         */
        static auto
        make(const std::shared_ptr<spdlog::logger> logger, stream&& ws) -> std::shared_ptr<connection>
        {
            // We have to emulate make_shared here because the ctor is private
            connection* me = nullptr;
            try {
                me = new connection{logger, std::move(ws)};
                return std::shared_ptr<connection>{me};
            } catch (...) {
                delete me;
                throw;
            }
        }
        /**
         * @brief Connect to a remote (websocket) endpoint
         * @note Only available if isClient == true
         * @param target The list of resolved endpoints to connect to
         * @param resource A suburl to make the connection on (e.g. `/api/websocket`)
         * @param done Callback invoked on accepting the handshake or an error
         * occurring
         */
        template<concepts::accept_handler Callback>
        void
        connect(const boost::asio::ip::tcp::resolver::results_type& target, const std::string& resource, Callback&& done) requires(isClient)
        {
            // Save these for later

            // Set the timeout for the operation
            m_ws.get_lowest_layer([&, me = this->shared_from_this(), this, done = std::forward<Callback>(done), resource](auto& sock) mutable {
                sock.expires_after(std::chrono::seconds(30));

                // Make the connection on the IP address we get from a lookup
                sock.async_connect(
                    target,
                    [me, target, done = std::forward<Callback>(done), resource](auto ec, auto ep) mutable {
                        if (ec) {
                            done(ec);
                        } else {
                            me->on_connect(ec, ep, resource, std::forward<decltype(done)>(done));
                        }
                    });
            });
        }

        /**
         * @brief Accept an incoming connection
         * @note This is only available if isClient == false
         * @note Calling this function is only valid if `make` was passed a
         * connected stream ready to be accepted (e.g. http stream that `req` was
         * read from)
         * @warning `done` is only invoked on successful connection
         * @param req The request to accept. boost::beast::websocket::is_upgrade(req) must be true
         * @param done Callback invoked on successful accepting of the request. NOT
         * invoked on failure. Must be invocable without any parameters (i.e. `f()`)
         */
        template<class Body, class Fields, std::invocable<> Callback>
        void
        accept(const boost::beast::http::request<Body, Fields>& req, Callback&& done) requires(!isClient)
        {
            // Update state
            m_state = state::handshaking;


            m_logger->trace("do_accept()");

            setup_connection();

            // Accept the websocket handshake
            m_ws.async_accept(req, [this, me = this->shared_from_this(), done = std::forward<decltype(done)>(done)](malloy::error_code ec) mutable {
                m_logger->trace("on_accept()");

                // Check for errors
                if (ec) {
                    m_logger->error("on_accept(): {}", ec.message());
                    return;
                }

                // We're good to go
                m_state = state::active;

                std::invoke(std::forward<decltype(done)>(done));
            });
        }

        // ToDo: Test this
        /**
         * @brief Stop/close the connection.
         * @note Attempting to send or receive after calling this will result in
         * error(s)
         *
         */
        void stop()
        {
            m_state = state::closing;

            m_ws.close();
            on_close();
        }

        /**
         * @brief Read a complete message into a buffer
         * @warning The caller is responsible for keeping the memory used by `buff`
         * alive until `done` is called
         *
         * @param buff Buffer to put the message into. Must satisfy dynamic_buffer @ref general_concepts
         * @param done Callback invoked on the message being read
         * (successfully or otherwise). Must satisfy async_read_handler @ref
         * general_concepts. Will NOT be invoked on the websocket being closed
         * before the message could be fully or partially read
         *
         */
        void
        read(concepts::dynamic_buffer auto& buff, concepts::async_read_handler auto&& done)
        {
            m_ws.async_read(buff, [this, me = this->shared_from_this(), done = std::forward<decltype(done)>(done)](auto ec, auto size) mutable {
                // This indicates that the connection was closed
                if (ec == boost::beast::websocket::error::closed) {
                    m_logger->info("on_read(): connection was closed.");
                    stop();
                    return;
                }
                std::invoke(std::forward<decltype(done)>(done), ec, size);
            });
        }

        /**
         * @brief Send the contents of a buffer to the client.
         * @warning The caller is responsible for keeping the memory of `payload`
         * alive until `done` is invoked
         *
         * @param payload The payload to send. Must satisfy const_buffer_sequence
         *  @ref general_concepts
         * @param done Callback invoked after the message is written (successfully
         * or otherwise). Must satisfy async_read_handler @ref general_concepts
         */
        template<concepts::async_read_handler Callback>
        void
        send(const concepts::const_buffer_sequence auto& payload, Callback&& done)
        {
            m_logger->trace("send(). payload size: {}", payload.size());

            boost::asio::post(
                m_ws.get_executor(),
                [this, payload, me = this->shared_from_this(), done = std::forward<Callback>(done)]() mutable {
                    msg_queue_.push_back([this, me, payload, done = std::forward<Callback>(done)]() mutable { me->m_ws.async_write(payload,
                                                                                                                                   [me, done = std::forward<Callback>(done)](auto ec, auto size) mutable {
                                                                                                                                       std::invoke(std::forward<Callback>(done), ec, size);
                                                                                                                                       me->on_write(ec, size);
                                                                                                                                   }); });

                    if (msg_queue_.size() > 1) {
                        return;
                    } else {
                        msg_queue_.back()();    // Execute this now
                    }
                });
        };

    private:
        enum class sending_state
        {
            idling,
            sending
        };

        enum sending_state m_sending_state = sending_state::idling;

        std::vector<std::function<void()>> msg_queue_;
        std::shared_ptr<spdlog::logger> m_logger;
        stream m_ws;

        enum state m_state = state::closed;

        connection(
            std::shared_ptr<spdlog::logger> logger, stream&& ws) :
            m_logger(std::move(logger)),
            m_ws{std::move(ws)}
        {
            // Sanity check logger
            if (!m_logger)
                throw std::invalid_argument("no valid logger provided.");
        }

        void
        setup_connection()
        {
            // Set suggested timeout settings for the websocket
            m_ws.set_option(
                boost::beast::websocket::stream_base::timeout::suggested(
                    isClient ? boost::beast::role_type::client : boost::beast::role_type::server));

            if constexpr (isClient) {
                // Set a decorator to change the User-Agent of the handshake
                m_ws.set_option(
                    boost::beast::websocket::stream_base::decorator(
                        [](boost::beast::websocket::request_type& req) {
                            req.set(boost::beast::http::field::user_agent, agent_string);
                        }));
            } else {
                // Set a decorator to change the Server of the handshake
                m_ws.set_option(
                    boost::beast::websocket::stream_base::decorator(
                        [](boost::beast::websocket::response_type& res) {
                            res.set(boost::beast::http::field::server, agent_string);
                        }));
            }
        }

        void
        on_connect(
            boost::beast::error_code ec,
            boost::asio::ip::tcp::resolver::results_type::endpoint_type ep,
            const std::string& resource,
            concepts::accept_handler auto&& on_handshake)
        {
            m_logger->trace("on_connect()");

            if (ec) {
                m_logger->error("on_connect(): {}", ec.message());
                return;
            }

            // Update the m_host string. This will provide the value of the
            // Host HTTP header during the WebSocket handshake.
            // See https://tools.ietf.org/html/rfc7230#section-5.4
            const std::string host = fmt::format("{}:{}", ep.address().to_string(), ep.port());


#if MALLOY_FEATURE_TLS
            if constexpr (isClient) {
                if (m_ws.is_tls()) {
                    // TODO: Should this be a seperate method?
                    m_ws.async_handshake_tls(boost::asio::ssl::stream_base::handshake_type::client, [on_handshake = std::forward<decltype(on_handshake)>(on_handshake),
                                                                                                     resource, host, me = this->shared_from_this()](auto ec) mutable {
                        if (ec) {
                            on_handshake(ec);
                        }

                        me->on_ready_for_handshake(host, resource, std::forward<decltype(on_handshake)>(on_handshake));
                    });
                    return;
                }
            }
#endif
            on_ready_for_handshake(host, resource, std::forward<decltype(on_handshake)>(on_handshake));
        }
        void on_ready_for_handshake(const std::string& host, const std::string& resource, concepts::accept_handler auto&& on_handshake)
        {
            // Turn off the timeout on the tcp_stream, because
            // the websocket stream has its own timeout system.
            m_ws.get_lowest_layer([](auto& s) { s.expires_never(); });
            setup_connection();


            // Perform the websocket handshake
            m_ws.async_handshake(
                host,
                resource,
                std::forward<decltype(on_handshake)>(on_handshake));
        }

        void
        on_write(auto ec, auto size)
        {
            if (ec) {
                m_logger->error("on_write failed for websocket connection: '{}'", ec.message());
                return;
            }

            m_logger->trace("on_write() wrote: '{}' bytes", size);

            assert(!msg_queue_.empty());

            msg_queue_.erase(msg_queue_.begin());

            if (!msg_queue_.empty())
                msg_queue_.front()();
        }

        void
        on_close()
        {
            m_logger->trace("on_close()");

            m_state = state::closed;
        }
    };

}    // namespace malloy::websocket
