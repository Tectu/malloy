
#pragma once

#include <concepts>

#include <boost/beast/error.hpp>
#include <boost/asio/io_context.hpp>

#include "malloy/type_traits.hpp"

namespace malloy::websockets {

template<bool isClient> 
class connection {
    using namespace net = boost::asio;
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
    static const std::string agent_string;

    connection(
        std::shared_ptr<spdlog::logger> logger
    ) :
        m_logger(std::move(logger))
    {
        // Sanity check logger
        if (!m_logger)
            throw std::invalid_argument("no valid logger provided.");
    }

    void set_handler(handler_t&& handler)
    {
        m_handler = std::move(handler);
    }

    // Start the asynchronous operation
    template<class Body, class Fields>
    void
        async_accept(boost::beast::http::request<Body, Fields> req, concepts::accept_handler auto&& done)
    {
        // Update state
        m_state = state::handshaking;


        m_logger->trace("do_accept()");

        // Set suggested timeout settings for the websocket
        ws_.set_option(
            boost::beast::websocket::stream_base::timeout::suggested(
                isClient ? boost::beast::role_type::client : boost::beast::role_type::server
            )
        );

        // Set a decorator to change the Server of the handshake
        derived().stream().set_option(
            boost::beast::websocket::stream_base::decorator(
                [](boost::beast::websocket::response_type& res)
                {
                    res.set(boost::beast::http::field::server, agent_string);
                }
            )
        );

        // Accept the websocket handshake
        ws_.async_accept(
            req,
            [me = this->shared_from_this(), done = std::forward<decltype(done)>(done)](auto ec){
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

        boost::asio::dispatch(
            boost::beast::bind_front_handler(
                &connection::on_close(),
                derived().shared_from_this()
            )
        );
    }
    void async_read(concepts::dynamic_buffer auto& buff, concepts::async_read_handler auto&& done) {
        ws_.async_read(buff, std::forward<decltype(done)>(done));
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
        )
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
        std::queue<std::string> m_tx_queue;
        net::io_context::strand ioc_sync_;
        stream ws_;

        enum state m_state = state::closed;

                void
        on_accept(boost::beast::error_code ec)
        {
        }

        void
        initiate_rx()
        {
            m_logger->trace("do_read()");

            assert(m_state == state::active);

            // Read a message into our buffer
            derived().stream().async_read(
                m_buffer,
                boost::beast::bind_front_handler(
                    &connection::on_read,
                    derived().shared_from_this()
                )
            );
        }

        void
        on_read(
            boost::beast::error_code ec,
            std::size_t bytes_transferred
        )
        {
            m_logger->trace("on_read(): bytes read: {}", bytes_transferred);

            // This indicates that the connection was closed
            if (ec == boost::beast::websocket::error::closed) {
                m_logger->info("on_read(): connection was closed.");
                return;
            }

            // Check for errors
            if (ec) {
                m_logger->error("on_read(): {}", ec.message());
                return;
            }

            // Nothing to do if we didn't read anything
            if (bytes_transferred == 0)
                return;

            // Convert to string
            std::string payload = boost::beast::buffers_to_string(m_buffer.cdata());
            if (payload.empty())
                return;

            // Consume the buffer
            m_buffer.consume(m_buffer.size());

            // Handle
            if (m_handler)
                m_handler(payload, [this](const malloy::websocket::payload_t& resp) {
                    send(resp);
                });

            // Do more reading
            initiate_rx();
        }

        void
        maybe_send_next()
        {
            m_logger->trace("maybe_send_next()");

            if (m_state != state::active)
                return;
            if (m_sending_state == sending_state::sending)
                return;
            if (m_tx_queue.empty())
                return;

            initiate_tx();
        }

        void
        initiate_tx()
        {
            m_logger->trace("initiate_tx()");

            assert(m_sending_state == sending_state::idling);
            assert(!m_tx_queue.empty());

            m_sending_state = sending_state::sending;
            derived().stream().async_write(
                boost::asio::buffer(m_tx_queue.front()),
                [self = derived().shared_from_this()](const boost::beast::error_code& ec, std::size_t) {
                    // we don't care about bytes_transferred
                    self->handle_tx(ec);
                });
        }

        void
        handle_tx(const boost::beast::error_code& ec)
        {
            m_logger->trace("handle_tx()");

            if (ec) {
                m_logger->error("failed to send message: {}", ec.message());
                return;
            }

            m_tx_queue.pop();
            m_sending_state = sending_state::idling;
            maybe_send_next();
        }

        void
        on_close()
        {
            m_logger->trace("on_close()");

            m_state = state::closed;
        }
    };

    template<typename Derived>
    const std::string connection<Derived>::agent_string = std::string(BOOST_BEAST_VERSION_STRING) + " malloy";


};

}
