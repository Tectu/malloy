#include "../../example_logger.hpp"

#include <malloy/server/controller.hpp>
#include <malloy/http/generator.hpp>
#include <malloy/server/routing/router.hpp>
#include <malloy/error.hpp>

#include <iostream>
#include <memory>

class ws_echo : public std::enable_shared_from_this<ws_echo> {
public:
    using conn_t = std::shared_ptr<malloy::server::websocket::connection>;
    ws_echo(const malloy::http::request<>& req, conn_t conn) : conn_{ std::move(conn) } {
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


int main()
{
    const std::filesystem::path doc_root = "../../../../examples/server/static_content";

    // Create malloy controller config
    malloy::server::controller::config cfg;
    cfg.interface   = "127.0.0.1";
    cfg.port        = 8080;
    cfg.doc_root    = doc_root;
    cfg.num_threads = 5;
    cfg.logger      = create_example_logger();

    // Create malloy controller
    malloy::server::controller c;
    if (!c.init(cfg)) {
        std::cerr << "could not start controller." << std::endl;
        return EXIT_FAILURE;
    }

    // Create the router
    auto router = c.router();
    if (router) {
        using namespace malloy::http;

        // A simple GET route handler
        router->add(method::get, "/", [](const auto& req) {
            response res{status::ok};
            res.body() = "<html><body><h1>Hello World!</h1><p>some content...</p></body></html>";
            return res;
        });

        // Add a route to an existing file
        router->add(method::get, "/file", [doc_root](const auto& req) {
            return generator::file(doc_root, "index.html");
        });

        // Add a route to a non-existing file
        router->add(method::get, "/file_nonexist", [doc_root](const auto& req) {
            return generator::file(doc_root, "/some_nonexisting_file.xzy");
        });

        // Add some redirections
        router->add_redirect(status::permanent_redirect, "/redirect1", "/");
        router->add_redirect(status::temporary_redirect, "/redirect2", "/");

        // Add some file serving
        router->add_file_serving("/files", doc_root);

        // Add a websocket echo endpoint
        router->add_websocket("/echo", [](const auto& req, auto writer) {
            
            std::make_shared<ws_echo>(req, writer);
            });
    }

    // Start
    c.start();

    // Keep the application alive
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    return EXIT_SUCCESS;
}
