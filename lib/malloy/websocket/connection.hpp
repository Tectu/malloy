
#pragma once

#include <concepts>
#include <memory>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

#include <boost/beast/core/error.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>


#include "malloy/type_traits.hpp"
#include "malloy/websocket/stream.hpp"
#include "malloy/websocket/types.hpp"
#include "malloy/error.hpp"

namespace malloy::websocket {

template<bool isClient> 
class connection : public std::enable_shared_from_this<connection<isClient>> {
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
    static const std::string agent_string;

    static auto make(const std::shared_ptr<spdlog::logger> logger, stream&& ws) -> std::shared_ptr<connection> requires (!isClient) {
        // We have to emulate make_shared here because the ctor is private
        connection* me = nullptr;
        try {
            me = new connection{ logger, std::move(ws) };
            return std::shared_ptr<connection>{ me };
        }
        catch (...) {
            delete me;
            throw;
        }
    }
    // Start the asynchronous operation
    template<concepts::accept_handler Callback>
        requires (isClient)
    void
            connect(const boost::asio::ip::tcp::resolver::results_type& target, const std::string& resource, Callback&& done)
        {

            // Save these for later

            // Set the timeout for the operation
            ws_.get_lowest_layer([&, this, done = std::forward<Callback>(done)](auto& sock) mutable {
                sock.expires_after(std::chrono::seconds(30));

                // Make the connection on the IP address we get from a lookup
                sock.async_connect(
                    target,
                    [me = this->shared_from_this(), target, done = std::forward<Callback>(done)](auto ec, auto ep) mutable {
                    if (ec) {
                        done(ec);
                    }
                    else {
                        me->on_connect(ec, ep, resource, std::forward<decltype(done)>(done));
                    }
                }
                );
                });
        }

    // Start the asynchronous operation
    template<class Body, class Fields, std::invocable<> Callback>
    requires (!isClient)
        void accept(const boost::beast::http::request<Body, Fields>& req, Callback&& done)
    {
        // Update state
        m_state = state::handshaking;


        m_logger->trace("do_accept()");

        setup_connection();

        // Accept the websocket handshake
        ws_.async_accept(req, [this, me = this->shared_from_this(), done = std::forward<decltype(done)>(done)](malloy::error_code ec) mutable {
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
    void stop()
    {
        m_state = state::closing;

        ws_.close();
        on_close();
    }
    void read(concepts::dynamic_buffer auto& buff, concepts::async_read_handler auto&& done) {
        ws_.async_read(buff, [this, me = this->shared_from_this(), done = std::forward<decltype(done)>(done)](auto ec, auto size) mutable {
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
     * Send a payload to the client.
     *
     * @param payload The payload to send.
     */
    template<std::invocable<> Callback>
    void send(const concepts::const_buffer_sequence auto& payload, Callback&& done)
    {
        m_logger->trace("send(). payload size: {}", payload.size());

        boost::asio::post(
            ws_.get_executor(),
            [this, payload, me = this->shared_from_this(), done = std::forward<Callback>(done)]() mutable {
            msg_queue_.push_back([this, me, payload, done = std::forward<Callback>(done)]() mutable {
                me->ws_.async_write(payload,
                [me, done = std::forward<Callback>(done)](auto ec, auto size) mutable {
                me->on_write(ec, size); 
                done();
            }); }
            );

            if (msg_queue_.size() > 1) {
                return;
            }
            else {
                msg_queue_.back()(); // Execute this now
            }
        }
        );
    }

private:
    enum class sending_state
    {
        idling,
        sending
    };

    enum sending_state m_sending_state = sending_state::idling;

    std::vector<std::function<void()>> msg_queue_;
    std::shared_ptr<spdlog::logger> m_logger;
    stream ws_;

    enum state m_state = state::closed;

    connection(
        std::shared_ptr<spdlog::logger> logger, stream&& ws
    ) :
        m_logger(std::move(logger)),
        ws_{ std::move(ws) }
    {
        // Sanity check logger
        if (!m_logger)
            throw std::invalid_argument("no valid logger provided.");
    }

    void setup_connection() {
        // Set suggested timeout settings for the websocket
        ws_.set_option(
            boost::beast::websocket::stream_base::timeout::suggested(
                isClient ? boost::beast::role_type::client : boost::beast::role_type::server
            )
        );

        if constexpr (isClient) {
            // Set a decorator to change the User-Agent of the handshake
            ws_.set_option(
                boost::beast::websocket::stream_base::decorator(
                    [](boost::beast::websocket::request_type& req)
                    {
                        req.set(boost::beast::http::field::user_agent, agent_string);
                    }
                )
            );
        }
        else {

            // Set a decorator to change the Server of the handshake
            ws_.set_option(
                boost::beast::websocket::stream_base::decorator(
                    [](boost::beast::websocket::response_type& res)
                    {
                        res.set(boost::beast::http::field::server, agent_string);
                    }
                )
            );
        }
    }

    void
        on_connect(
            boost::beast::error_code ec, 
            boost::asio::ip::tcp::resolver::results_type::endpoint_type ep, 
            const std::string& resource,
            concepts::accept_handler auto&& on_handshake
        )
    {
        m_logger->trace("on_connect()");

        if (ec) {
            m_logger->error("on_connect(): {}", ec.message());
            return;
        }

        // Turn off the timeout on the tcp_stream, because
        // the websocket stream has its own timeout system.
        ws_.get_lowest_layer([](auto& s) { s.expires_never(); });
        setup_connection();

      

        // Update the m_host string. This will provide the value of the
        // Host HTTP header during the WebSocket handshake.
        // See https://tools.ietf.org/html/rfc7230#section-5.4
        const std::string host = fmt::format("{}:{}", ep.address().string(), ep.port());

        // Perform the websocket handshake
        ws_.async_handshake(
            host,
            resource,
            std::forward<decltype(on_handshake)>(on_handshake)
        );
    }

    void on_write(auto ec, auto size) {
        if (ec) {
            m_logger->error("on_write failed for websocket connection: '{}'", ec.message());
            return;
        }
        m_logger->trace("on_write() wrote: '{}' bytes", size);

        assert(!msg_queue_.empty());

        msg_queue_.erase(msg_queue_.begin());

        if (!msg_queue_.empty()) {
            msg_queue_.front()();
        }


    }

    void
        on_close()
    {
        m_logger->trace("on_close()");

        m_state = state::closed;
    }
};

template<>
const std::string connection<true>::agent_string = std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-async";

template<>
const std::string connection<false>::agent_string = std::string{BOOST_BEAST_VERSION_STRING} + " malloy";

}
