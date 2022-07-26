#pragma once

#include <malloy/core/utils.hpp>
#include <malloy/core/http/request.hpp>
#include <malloy/core/error.hpp>
#include <malloy/core/websocket/connection.hpp>

#include <fmt/format.h>

#include <thread>

namespace malloy::examples::ws
{

    namespace detail
    {
        // Alias for easy referencing
        template<bool isClient>
        using ws_connection = std::shared_ptr<malloy::websocket::connection<isClient>>;
    }

    /**
     * @brief Reads a single message from the connection
     *
     * @param on_read Callback invoked on a message being read or an error occurring. Must be invocable with malloy::error_code, std::string
     */
    template<bool isClient>
    void
    oneshot_read(const detail::ws_connection<isClient>& conn, std::invocable<malloy::error_code, std::string> auto&& on_read)
    {
        auto buffer = std::make_shared<boost::beast::flat_buffer>();
        conn->read(
            *buffer,
            [buffer, on_read = std::forward<decltype(on_read)>(on_read)](auto ec, auto) {
                on_read(ec, malloy::buffers_to_string(malloy::buffer(buffer->cdata(), buffer->size())));
            }
        );
    }

    /**
     * @brief Accepts a connection and sends a single message
     *
     * @note Copies `msg_data` into heap-allocated memory
     * @param req Request to accept
     * @param conn Connection to use
     * @param msg_data Message to send
     */
    template<bool isClient>
    void accept_and_send(const auto& req, detail::ws_connection<isClient> conn, const std::string& msg_data)
    {
        auto msg = std::make_shared<std::string>(msg_data); // Keep message memory alive
        conn->accept(
            req,
            [msg, conn] {
                conn->send(malloy::buffer(*msg), [msg](auto, auto){ });
            }
        );
    }

    template<bool isClient>
    struct read_until_disconnect
    {
        using conn_t = std::shared_ptr<malloy::websocket::connection<isClient>>;

        explicit
        read_until_disconnect(conn_t conn) :
            m_conn{ std::move(conn) }
        {
        }

        void
        run(const malloy::http::request<>& req)
        {
            m_conn->accept(req, boost::beast::bind_front_handler(&read_until_disconnect::do_read, this->shared_from_this()));
        }
    private:
        conn_t conn;

        void
        on_read(malloy::error_code ec, std::size_t)
        {
            // The connection was closed by the client
            if (ec == malloy::websocket::error::closed)
                return;
        }
    };

    template<bool isClient>
    class ws_echo :
        public std::enable_shared_from_this<ws_echo<isClient>>
    {
    public:
        using conn_t = std::shared_ptr<malloy::websocket::connection<isClient>>;

        explicit
        ws_echo(conn_t conn) :
            m_conn{ std::move(conn) }
        {
        }

        void
        run(const malloy::http::request<>& req)
        {
            m_conn->accept(req, boost::beast::bind_front_handler(&ws_echo::do_read, this->shared_from_this()));
        }

    private:
        conn_t m_conn;
        boost::beast::flat_buffer m_buffer;

        void
        on_read(malloy::error_code ec, std::size_t)
        {
            // The connection was closed by the client
            if (ec == malloy::websocket::error::closed)
                return;

            if (ec) {
                spdlog::error("oh no, I couldn't read: '{}'", ec.message());
                return;
            }

            m_conn->send(
                boost::asio::buffer(m_buffer.cdata(), m_buffer.size()),
                boost::beast::bind_front_handler(&ws_echo::on_write, this->shared_from_this())
            );
        }

        void
        on_write(malloy::error_code ec, std::size_t amount)
        {
            if (ec)
                spdlog::error("I failed to write: '{}'", ec.message());
            else if (amount != m_buffer.size())
                spdlog::error("Oh snap! We could not write the full buffer ({} out of {})", amount, m_buffer.size());
            else {
                m_buffer.consume(amount);
                do_read();
            }
        }

        void
        do_read()
        {
            m_conn->read(m_buffer, boost::beast::bind_front_handler(&ws_echo::on_read, this->shared_from_this()));
        }
    };

    using server_echo = ws_echo<false>;

    /**
     * @brief Provides a timer over websocket.
     *
     * @details Will send i = n for n in range 0..count inclusive at an interval of 1
     *          second and then close the connection (it counts from 0 to 9 with a second between each message)
     */
    template<bool isClient, size_t count = 5>
    class ws_timer :
        public std::enable_shared_from_this<ws_timer<isClient>>
    {
    public:
        explicit
        ws_timer(detail::ws_connection<isClient> conn) :
            m_conn{ std::move(conn) }
        {
            static_assert(count > 0);
        }

        template<typename Req>
        void
        run(const Req& request)
        {
            m_conn->accept(request, malloy::bind_front_handler(&ws_timer::do_write, this->shared_from_this()));
        }

    private:
        boost::beast::flat_buffer m_buffer;
        detail::ws_connection<isClient> m_conn;
        int m_wrote_secs{ 0 };
        std::array<std::string, count> m_msg_store; // Keeps sent messages data alive

        void
        do_read()
        {
            m_conn->read(m_buffer, malloy::bind_front_handler(&ws_timer::on_read, this->shared_from_this()));
        }

        void
        on_write(malloy::error_code ec, std::size_t bytes)
        {
            if (ec)
                spdlog::error("Uh oh, I couldn't write: '{}'", ec.message());

            if (m_wrote_secs == count-1)
                m_conn->disconnect(); // Kill the connection, we've finished our job
        }

        void
        do_write()
        {
            using namespace std::chrono_literals;

            for (std::size_t i = 0; i < count; i++) {
                // Write to socket
                m_msg_store[i] = fmt::format("i = {}", i);
                m_conn->send(malloy::buffer(m_msg_store[i]), [this, me = this->shared_from_this()](auto ec, auto size) {
                    me->on_write(ec, size);
                    ++m_wrote_secs;

                    std::this_thread::sleep_for(1s);
                });
            }
        }

        void
        on_read(malloy::error_code ec, std::size_t bytes)
        {
            if (ec) {
                spdlog::error("Uh oh, I couldn't read: '{}'", ec.message());
                return;
            }

            do_write();
        }
    };

    using server_timer = ws_timer<false>;

}
