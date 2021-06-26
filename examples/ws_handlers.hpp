
#pragma once

#include <spdlog/fmt/fmt.h>

#include <malloy/utils.hpp>
#include <malloy/error.hpp>

#include <malloy/websocket/connection.hpp>

namespace malloy::examples::ws {
using ws_connection = std::shared_ptr<malloy::websocket::connection>;


void oneshot_read(const ws_connection& conn, std::invocable<malloy::error_code, std::string> auto&& on_read) {
    auto buffer = std::make_shared<boost::beast::flat_buffer>();
    conn->read(*buffer, [buffer, on_read = std::forward<decltype(on_read)>(on_read)](auto ec, auto) {
        on_read(ec, malloy::buffers_to_string(*buffer));
    });
}

void oneshot(const auto& req, ws_connection conn, const std::string& msg) {
    conn->accept(req, [msg, conn] { conn->send(malloy::buffer(msg), [](auto, auto) {}); });
}

class ws_echo : public std::enable_shared_from_this<ws_echo> {
public:
    using conn_t = std::shared_ptr<malloy::server::websocket::connection>;
    explicit ws_echo(const malloy::http::request<>& req, conn_t conn) : conn_{ std::move(conn) } {
        conn_->accept(req, boost::beast::bind_front_handler(&ws_echo::do_read, shared_from_this()));
    }


private:
    void on_read(malloy::error_code ec, std::size_t) {
        if (ec) {
            spdlog::error("oh no, I couldn't read: '{}'", ec.message());
            return;
        }
        conn_->send(boost::asio::buffer(buff_.cdata()), boost::beast::bind_front_handler(&ws_echo::do_read, shared_from_this()));
    }
    void do_read() {
        buff_.consume(buff_.size());
        conn_->read(buff_, boost::beast::bind_front_handler(&ws_echo::on_read, shared_from_this()));

    }

    conn_t conn_;
    boost::beast::flat_buffer buff_;


};



class ws_timer: public std::enable_shared_from_this<ws_timer> {
public:
    explicit ws_timer(ws_connection conn) : conn_{ std::move(conn) } {}
    template<typename Req>
    void run(const Req& request) {
        conn_->accept(request, malloy::bind_front_handler(&ws_timer::do_read, shared_from_this()));
    }
private:
    void do_read() {
        conn_->read(buff_, malloy::bind_front_handler(&ws_timer::on_read, shared_from_this()));
    }
    void on_write(malloy::error_code ec, std::size_t bytes) {
        if (ec) {
            spdlog::error("Uh oh, I couldn't write: '{}'", ec.message());
        }
    }
    void on_read(malloy::error_code ec, std::size_t bytes) {
        using namespace std::chrono_literals;
        if (ec) {
            spdlog::error("Uh oh, I couldn't read: '{}'", ec.message());
            return;
        }
        for (std::size_t i = 0; i < 10; i++) {
            // Write to socket
            conn_->send(fmt::format("i = {}", i), malloy::bind_front_handler(&ws_timer::on_write, shared_from_this()));
            // Sleep
            std::this_thread::sleep_for(1s);
        }
        do_read();
    }

    boost::beast::flat_buffer buff_;
    ws_connection conn_;
};


}
