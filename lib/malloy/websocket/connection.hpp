
#pragma once

#include <concepts>
#include <memory>

#include <spdlog/spdlog.h>

#include <boost/beast/core/error.hpp>
#include <boost/asio/io_context.hpp>

#include "malloy/type_traits.hpp"
#include "malloy/websocket/stream.hpp"

namespace malloy::websocket {

template<bool isClient> 
class connection : public std::enable_shared_from_this<connection<isClient>> {
public:
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
    inline static const std::string agent_string = std::string{BOOST_BEAST_VERSION_STRING} + " malloy";

    static auto make(const std::shared_ptr<spdlog::logger> logger, stream&& ws) -> std::shared_ptr<connection> {
        // We have to emulate make_shared here because the ctor is private
        connection* me = nullptr;
        try {
            me = new connection{ logger, std::move(ws) };
            return std::shared_ptr{ me };
        }
        catch (...) {
            delete me;
            throw;
        }
    }
    
    // Start the asynchronous operation
    template<class Body, class Fields>
    requires (!isClient)
        void accept(boost::beast::http::request<Body, Fields> req, concepts::accept_handler auto&& done)
    {
        // Update state
        m_state = state::handshaking;


        m_logger->trace("do_accept()");

        // Set suggested timeout settings for the websocket
        ws_.set_option(
            boost::beast::websocket::stream_base::timeout::suggested(
                boost::beast::role_type::server
            )
        );

        // Set a decorator to change the Server of the handshake
        ws_.set_option(
            boost::beast::websocket::stream_base::decorator(
                [](boost::beast::websocket::response_type& res)
                {
                    res.set(boost::beast::http::field::server, agent_string);
                }
            )
        );

        // Accept the websocket handshake
        ioc_sync_.post([me = this->shared_from_this(), req, done = std::forward<decltype(done)>(done)]{
                const auto ec = ws_.accept(req);
                m_logger->trace("on_accept()");

                // Check for errors
                if (ec) {
                    m_logger->error("on_accept(): {}", ec.message());
                    return;
                }

                // We're good to go
                m_state = state::active;

                done(ec);
            });
    }

    // ToDo: Test this
    void stop()
    {
        m_state = state::closing;

        ioc_sync_.post([me = this->shared_from_this()]{
            me->on_close();
            });
    }
    void read(concepts::dynamic_buffer auto& buff, concepts::async_read_handler auto&& done) {
        ioc_sync_.post([me = this->shared_from_this(), done = std::forward<decltype(done)>(done), buff]{
            boost::beast::error_code ec;
            const auto size = ws_.read(buff, ec);
            // This indicates that the connection was closed
            if (ec == boost::beast::websocket::error::closed) {
                m_logger->info("on_read(): connection was closed.");
                stop();
                return;
            }
            std::invoke(done, ec, size);
            });
    }

    /**
     * Send a payload to the client.
     *
     * @param payload The payload to send.
     */
    void send(const concepts::const_buffer_sequence auto& payload)
    {
        m_logger->trace("send(). payload size: {}", payload.size());

        ioc_sync_.post(
            [payload]() mutable {
                ws_.write(payload);
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

    std::shared_ptr<spdlog::logger> m_logger;
    boost::asio::io_context::strand ioc_sync_;
    stream ws_;

    enum state m_state = state::closed;

    connection(
        std::shared_ptr<spdlog::logger> logger, stream&& ws
    ) :
        m_logger(std::move(logger)),
        ws_{std::move(ws)}
    {
        // Sanity check logger
        if (!m_logger)
            throw std::invalid_argument("no valid logger provided.");
    }



    void
        on_close()
    {
        m_logger->trace("on_close()");

        m_state = state::closed;
    }
};

}
