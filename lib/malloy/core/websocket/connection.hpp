#pragma once

#include "types.hpp"
#include "../error.hpp"
#include "../type_traits.hpp"
#include "../utils.hpp"
#include "../detail/action_queue.hpp"
#include "../http/request.hpp"
#include "../websocket/stream.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/beast/core/error.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <concepts>
#include <functional>
#include <memory>

namespace malloy::websocket
{

    /**
     * @class connection
     * @tparam isClient: Whether it is the client end of a websocket connection
     * @brief Represents a connection via the WebSocket protocol
     * @details Provides basic management of a websocket connection. Will close the
     * connection on destruction. The interface is entirely asynchronous and uses
     * callback functions
     * @note The functions: connect, accept, disconnect, send, and read are fully threadsafe. Calls to these functions will wait asynchronously until all other
     * queued actions are completed.
     */
    template<bool isClient>
    class connection :
        public std::enable_shared_from_this<connection<isClient>>
    {
        using ws_executor_t = std::invoke_result_t<decltype(&stream::get_executor), stream*>;
        using act_queue_t = malloy::detail::action_queue<ws_executor_t>;

    public:
        using handler_t = std::function<void(const malloy::http::request<>&, const std::shared_ptr<connection>&)>;

        /**
         * The connection state.
         */
        enum class state
        {
            handshaking,
            active,
            closing,
            closed,
            inactive, // Initial state
        };

        /**
         * Destructor.
         */
        virtual
        ~connection() noexcept
        {
            m_logger->trace("destructor()");
        }

        /**
         * Get the logger instance.
         *
         * @details This allows different components to log to the connection specific logger.
         *
         * @return The logger instance.
         */
        [[nodiscard]]
        std::shared_ptr<spdlog::logger>
        logger() const noexcept
        {
            return m_logger;
        }

        /**
         * See stream::set_binary(bool)
         */
        void
        set_binary(const bool enabled)
        {
            m_ws.set_binary(enabled);
        }

        /**
         * See stream::binary()
         */
        [[nodiscard]]
        bool
        binary()
        {
            return m_ws.binary();
        }

        /**
         * @brief Construct a new connection object
         * @param logger Logger to use. Must not be `nullptr`
         * @param ws Stream to use. May be unopened/connected but in that case
         * `connect` must be called before this connection can be used
         */
        static
        std::shared_ptr<connection>
        make(const std::shared_ptr<spdlog::logger> logger, stream&& ws, const std::string& agent_string)
        {
            // We have to emulate make_shared here because the ctor is private
            connection* me = nullptr;
            try {
                me = new connection{logger, std::move(ws), agent_string};
                return std::shared_ptr<connection>{me};
            }
            catch (...) {
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
        connect(const boost::asio::ip::tcp::resolver::results_type& target, const std::string& resource, Callback&& done)
            requires(isClient)
        {
            m_logger->trace("connect()");

            if (m_state != state::inactive)
                throw std::logic_error{"connect() called on already active websocket connection"};

            // Set the timeout for the operation
            m_ws.get_lowest_layer([&, me = this->shared_from_this(), this, done = std::forward<Callback>(done), resource](auto& sock) mutable {
                sock.expires_after(std::chrono::seconds(30));

                // Make the connection on the IP address we get from a lookup
                sock.async_connect(
                    target,
                    [this, me, target, done = std::forward<Callback>(done), resource](auto ec, auto ep) mutable {
                        if (ec) {
                            done(ec);
                        }
                        else {
                            me->on_connect(
                                ec,
                                ep,
                                resource,
                                [this, done = std::forward<Callback>(done)](auto ec) mutable {
                                    go_active();
                                    std::invoke(std::forward<decltype(done)>(done), ec);
                                }
                            );
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
        accept(const boost::beast::http::request<Body, Fields>& req, Callback&& done)
            requires(!isClient)
        {
            m_logger->trace("accept()");

            if (m_state != state::inactive)
                throw std::logic_error{"accept() called on already active websocket connection"};

            // Update state
            m_state = state::handshaking;

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
                go_active();

                std::invoke(std::forward<decltype(done)>(done));
            });
        }

        /**
         * @brief Disconnect/stop/close the connection.
         * @note Attempting to send or receive after calling this will result in error(s).
         * @note Calling this function will have no effect if the connection state is closed or closing.
         *
         * @param why Reason why the connection is being closed.
         *
         * @sa force_disconnect()
         */
        void
        disconnect(boost::beast::websocket::close_reason why = boost::beast::websocket::normal)
        {
            m_logger->trace("disconnect()");

            if (m_state == state::closed || m_state == state::closing)
                return;

            auto build_act = [this, why, me = this->shared_from_this()](const auto& on_done) mutable {
                // Check we haven't been beaten to it
                if (m_state == state::closed || m_state == state::closing) {
                    on_done();
                    return;
                }

                do_disconnect(why, on_done);
            };

            // We queue in both read and write, and whichever gets there first wins 
            m_write_queue.push(build_act);
            m_read_queue.push(build_act);
        }

        /**
         * @brief Same as disconnect, but bypasses all queues and runs immediately.
         * @note Attempting to send or receive after calling this will result in error(s).
         * @note Calling this function will have no effect if the connection state is closed or closing.
         *
         * @sa disconnect()
         */
        void
        force_disconnect(boost::beast::websocket::close_reason why = boost::beast::websocket::normal)
        {
            m_logger->trace("force_disconnect()");

            if (m_state == state::inactive)
                throw std::logic_error{"force_disconnect() called on inactive websocket connection"};

            else if (m_state == state::closed || m_state == state::closing)
                return; // Already disconnecting

            do_disconnect(why, []{});
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
            m_logger->trace("read()");

            m_read_queue.push(
                [
                    this,
                    me = this->shared_from_this(),
                    buff = &buff, // Capturing reference by value copies the object
                    done = std::forward<decltype(done)>(done)
                ]
                (const auto& on_done) mutable
                {
                    assert(buff != nullptr);
                    m_ws.async_read(*buff, [this, me, on_done, done = std::forward<decltype(done)>(done)](auto ec, auto size) mutable {
                        std::invoke(std::forward<decltype(done)>(done), ec, size);
                        on_done();
                    });
               }
           );
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

            m_write_queue.push([buff = payload, done = std::forward<Callback>(done), this, me = this->shared_from_this()](const auto& on_done) mutable {
                m_ws.async_write(buff, [this, me, on_done, done = std::forward<decltype(done)>(done)](auto ec, auto size) mutable {
                    on_write(ec, size);
                    std::invoke(std::forward<Callback>(done), ec, size);
                    on_done();
                });
            });
        }

    private:
        enum class sending_state
        {
            idling,
            sending
        };

        enum sending_state m_sending_state = sending_state::idling;
        std::shared_ptr<spdlog::logger> m_logger;
        stream m_ws;
        std::string m_agent_string;
        act_queue_t m_write_queue;
        act_queue_t m_read_queue;
        std::atomic<state> m_state{ state::inactive };

        connection(
            std::shared_ptr<spdlog::logger> logger, stream&& ws, std::string agent_str) :
            m_logger(std::move(logger)),
            m_ws{std::move(ws)},
            m_agent_string{std::move(agent_str)},
            m_write_queue{boost::asio::make_strand(m_ws.get_executor())},
            m_read_queue{boost::asio::make_strand(m_ws.get_executor())}
        {
            // Sanity check logger
            if (!m_logger)
                throw std::invalid_argument("no valid logger provided.");
        }
        
        void
        go_active()
        {
            m_logger->trace("go_active()");

            // Update state
            m_state = state::active;

            // Start/run action queues
            m_read_queue.run();
            m_write_queue.run();
        }

        void
        setup_connection()
        {
            m_logger->trace("setup_connection()");

            // Set suggested timeout settings for the websocket
            m_ws.set_option(
                boost::beast::websocket::stream_base::timeout::suggested(
                    isClient ? boost::beast::role_type::client : boost::beast::role_type::server)
            );

            // Set agent string/field
            const auto agent_field = isClient ? malloy::http::field::user_agent : malloy::http::field::server;
            m_ws.set_option(
                boost::beast::websocket::stream_base::decorator(
                    [this, agent_field](boost::beast::websocket::request_type& req) {
                        req.set(agent_field, m_agent_string);
                    }
                )
            );
        }

        void
        do_disconnect(boost::beast::websocket::close_reason why, const std::invocable<> auto& on_done)
        {
            m_logger->trace("do_disconnect()");

            // Update state
            m_state = state::closing;

            m_ws.async_close(why, [me = this->shared_from_this(), this, on_done](auto ec) {
                if (ec)
                    m_logger->error("could not close websocket: '{}'", ec.message());    // TODO: See #40
                else
                    on_close();

                on_done();
            });
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

            m_ws.get_lowest_layer([](auto& s) { s.expires_never(); }); // websocket has its own timeout system that conflicts

            // Update the m_host string. This will provide the value of the
            // Host HTTP header during the WebSocket handshake.
            // See https://tools.ietf.org/html/rfc7230#section-5.4
            const std::string host = fmt::format("{}:{}", ep.address().to_string(), ep.port());

#if MALLOY_FEATURE_TLS
            if constexpr (isClient) {
                if (m_ws.is_tls()) {
                    // TODO: Should this be a separate method?
                    m_ws.async_handshake_tls(
                        boost::asio::ssl::stream_base::handshake_type::client,
                        [on_handshake = std::forward<decltype(on_handshake)>(on_handshake), resource, host, me = this->shared_from_this()](auto ec) mutable
                        {
                            if (ec)
                                on_handshake(ec);

                            me->on_ready_for_handshake(host, resource, std::forward<decltype(on_handshake)>(on_handshake));
                        }
                    );
                    return;
                }
            }
#endif
            on_ready_for_handshake(host, resource, std::forward<decltype(on_handshake)>(on_handshake));
        }

        void
        on_ready_for_handshake(const std::string& host, const std::string& resource, concepts::accept_handler auto&& on_handshake)
        {
            m_logger->trace("on_ready_for_handshake()");

            // Turn off the timeout on the tcp_stream, because
            // the websocket stream has its own timeout system.
            m_ws.get_lowest_layer([](auto& s) { s.expires_never(); });
            setup_connection();

            // Perform the websocket handshake
            m_ws.async_handshake(
                host,
                resource,
                std::forward<decltype(on_handshake)>(on_handshake)
            );
        }

        void
        on_write(auto ec, auto size)
        {
            m_logger->trace("on_write() wrote: '{}' bytes", size);

            if (ec) {
                m_logger->error("on_write failed for websocket connection: '{}'", ec.message());
                return;
            }
        }

        void
        on_close()
        {
            m_logger->trace("on_close()");

            m_state = state::closed;
        }
    };

}    // namespace malloy::websocket
