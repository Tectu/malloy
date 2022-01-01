#pragma once

#include "../error.hpp"
#include "../http/request.hpp"
#include "../type_traits.hpp"
#include "../utils.hpp"
#include "../websocket/stream.hpp"
#include "malloy/core/detail/action_queue.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/beast/core/error.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <boost/beast/core.hpp>

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

        /**
         * @brief Expanded lambda for read(), allows support for non-copyable completion tokens
         */
        template<std::invocable<error_code, std::size_t> Callback, concepts::dynamic_buffer Buff>
        class read_delegate : public act_queue_t::action
        {
        public:
            Callback done;
            std::shared_ptr<connection> me;
            Buff* buff;

            template<typename Cb>
            requires(std::is_constructible_v<Callback, Cb>)
                read_delegate(Cb&& done, std::shared_ptr<connection> me, Buff* buff) :
                done{std::forward<Cb>(done)},
                me{std::move(me)}, buff{buff} {}

            void invoke(act_queue_t::act_args on_done) override
            {
                assert(buff != nullptr);
                me->m_ws.async_read(*buff, [me = me, &on_done, done = std::move(done)](auto ec, auto size) mutable {
                    // Warning: This is outside the lifetime of the delegate, do not capture [this]
                    std::invoke(std::move(done), ec, size);
                    on_done();
                });
            }
        };
        /**
         * @brief Same as read_delegate but for send()
         */
        template<std::invocable<error_code, std::size_t> Callback, concepts::const_buffer_sequence Payload>
        class write_delegate : public act_queue_t::action
        {
        public:
            Callback done;
            std::shared_ptr<connection> me;
            Payload buff;

            template<typename Cb>
            requires(std::is_constructible_v<Callback, Cb>)
                write_delegate(Cb&& done, std::shared_ptr<connection> me, Payload buff) :
                done{std::forward<Cb>(done)},
                me{std::move(me)}, buff{buff} {}

            void invoke(act_queue_t::act_args on_done) override
            {
                me->m_ws.async_write(buff, [me = me, &on_done, done = std::move(done)](auto ec, auto size) mutable {
                    // Warning: This is outside the lifetime of the delegate, do not capture [this]
                    me->on_write(ec, size);
                    std::invoke(std::move(done), ec, size);
                    on_done();
                });
            }
        };

        class disconnect_delegate : public act_queue_t::action {
        public:
            std::shared_ptr<connection> parent;
            boost::beast::websocket::close_reason why;

            disconnect_delegate(std::shared_ptr<connection> parent, boost::beast::websocket::close_reason why) : parent{std::move(parent)}, why{why} {}

            void invoke(act_queue_t::act_args on_done) override {
                // Check we haven't been beaten
                if (parent->m_state == state::closed || parent->m_state == state::closing) {
                    on_done();
                    return;
                }
                parent->do_disconnect(why, [on_done = &on_done]() { (*on_done)(); });
            }

        };

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
         * See stream::set_binary(bool)
         */
        void set_binary(const bool enabled) { m_ws.set_binary(enabled); }

        /**
         * See stream::binary()
         */
        [[nodiscard]] bool binary() { return m_ws.binary(); }

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
        template<concepts::err_completion_token Callback>
        auto
        connect(const boost::asio::ip::tcp::resolver::results_type& target, const std::string& resource, Callback&& done) requires(isClient)
        {
            if (m_state != state::inactive) {
                throw std::logic_error{"connect() called on already active websocket connection"};
            }
            auto me = this->shared_from_this();

            //boost::asio::async_completion<std::decay_t<Callback>, void(error_code)> comp{std::forward<Callback>(done)};
            auto on_sock_conn = [me, resource](auto act, auto ec, auto ep) {
                if (ec) {
                    std::invoke(std::move(act), ec);
                } else {
                    me->on_connect(ec, ep, resource, [me, act = std::move(act)](auto ec) mutable {
                        me->go_active();
                        std::invoke(std::move(act), ec);
                    });
                }
            };
            // Set the timeout for the operation
            return m_ws.get_lowest_layer([me, this, on_sock_conn = std::move(on_sock_conn), &target, resource, done = std::forward<Callback>(done)](auto& sock) mutable {
                sock.expires_after(std::chrono::seconds(30));

                // Make the connection on the IP address we get from a lookup
                auto act = [target, on_sock_conn, &sock](auto done) mutable {
                    sock.async_connect(target, [on_sock_conn, done = std::move(done)](auto ec, auto ep) mutable { on_sock_conn(std::move(done), ec, ep); });
                };
                return boost::asio::async_initiate<Callback, void(error_code)>(std::move(act), done);
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
        template<class Body, class Fields, boost::asio::completion_token_for<void()> Callback>
        auto
        accept(const boost::beast::http::request<Body, Fields>& req, Callback&& done) requires(!isClient)
        {
            if (m_state != state::inactive) {
                throw std::logic_error{"accept() called on already active websocket connection"};
            }

            // Update state
            m_state = state::handshaking;


            m_logger->trace("accept()");

            setup_connection();

            // Accept the websocket handshake
            auto wrapper = [req, this, me = this->shared_from_this()](auto done){
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
            };
            return boost::asio::async_initiate<Callback, void()>(std::move(wrapper), done);
        }

        /**
         * @brief Disconnect/stop/close the connection.
         * @note Attempting to send or receive after calling this will result in error(s).
         *
         * @param why Reason why the connection is being closed.
         *
         * @sa force_disconnect()
         */
        void
        disconnect(boost::beast::websocket::close_reason why = boost::beast::websocket::normal)
        {
            if (m_state == state::closed || m_state == state::closing) {
                throw std::logic_error{"disconnect() called on closed or closing websocket connection"};
            }
            auto build_act = [this, why]{ return disconnect_delegate{this->shared_from_this(), why}; };

            // We queue in both read and write, and whichever gets there first wins
            m_write_queue.push(build_act());
            m_read_queue.push(build_act());
        }

        /**
         * @brief Same as disconnect, but bypasses all queues and runs immediately
         *
         * @sa disconnect()
         */
        void
        force_disconnect(boost::beast::websocket::close_reason why = boost::beast::websocket::normal)
        {
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
        auto
        read(concepts::dynamic_buffer auto& buff, concepts::read_completion_token auto&& done)
        {
            using buff_t = std::remove_cvref_t<decltype(buff)>;
            m_logger->trace("read()");
            auto wrapper = [buff = &buff /* Capturing reference by value copies the object */, this, me = this->shared_from_this()](auto done) mutable {
                m_read_queue.push(read_delegate<std::remove_cvref_t<decltype(done)>, buff_t>{ std::move(done), me, buff});
            };
            return boost::asio::async_initiate<decltype(done), void(error_code, std::size_t)>(std::move(wrapper), done);
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
        template<concepts::read_completion_token Callback>
        auto
        send(const concepts::const_buffer_sequence auto& payload, Callback&& done)
        {
            using payload_t = std::remove_cvref_t<decltype(payload)>;
            m_logger->trace("send(). payload size: {}", payload.size());
            auto wrapper = [this, payload = payload, me = this->shared_from_this()](auto done) mutable {
                m_write_queue.push(write_delegate<std::remove_cv_t<decltype(done)>, payload_t>{std::move(done), me, payload });
            };
            return boost::asio::async_initiate<Callback, void(error_code, std::size_t)>(std::move(wrapper), done);
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

        std::atomic<state> m_state{state::inactive};

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
            m_state = state::active;
            m_read_queue.run();
            m_write_queue.run();
        }


        void
        setup_connection()
        {
            // Set suggested timeout settings for the websocket
            m_ws.set_option(
                boost::beast::websocket::stream_base::timeout::suggested(
                    isClient ? boost::beast::role_type::client : boost::beast::role_type::server));

            const auto agent_field = isClient ? malloy::http::field::user_agent : malloy::http::field::server;
            m_ws.set_option(
                boost::beast::websocket::stream_base::decorator(
                    [this, agent_field](boost::beast::websocket::request_type& req) {
                        req.set(agent_field, m_agent_string);
                    }));
        }

        void
        do_disconnect(boost::beast::websocket::close_reason why, const std::invocable<> auto& on_done)
        {
            // Update state
            m_state = state::closing;

            m_ws.async_close(why, [me = this->shared_from_this(), this, &on_done](auto ec){
                if (ec) {
                    m_logger->error("could not close websocket: '{}'", ec.message());    // TODO: See #40
                } else {
                    on_close();
                }
                on_done();
            });
        }

        template<concepts::err_completion_token Tkn>
        void
        on_connect(
            boost::beast::error_code ec,
            boost::asio::ip::tcp::resolver::results_type::endpoint_type ep,
            const std::string& resource,
            Tkn&& on_handshake)
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
                    // TODO: Should this be a seperate method?
                    m_ws.async_handshake_tls(boost::asio::ssl::stream_base::handshake_type::client, [on_handshake = std::forward<decltype(on_handshake)>(on_handshake),
                                                                                                     resource, host, me = this->shared_from_this()](auto ec) mutable {
                        if (ec)
                            on_handshake(ec);

                        me->on_ready_for_handshake(host, resource, std::forward<decltype(on_handshake)>(on_handshake));
                    });
                    return;
                }
            }
#endif
            on_ready_for_handshake(host, resource, std::forward<decltype(on_handshake)>(on_handshake));
        }

        auto
        on_ready_for_handshake(const std::string& host, const std::string& resource, concepts::err_completion_token auto&& on_handshake)
        {
            // Turn off the timeout on the tcp_stream, because
            // the websocket stream has its own timeout system.
            m_ws.get_lowest_layer([](auto& s) { s.expires_never(); });
            setup_connection();


            // Perform the websocket handshake
            return m_ws.async_handshake(
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

        }

        void
        on_close()
        {
            m_logger->trace("on_close()");

            m_state = state::closed;
        }
    };

}    // namespace malloy::websocket
