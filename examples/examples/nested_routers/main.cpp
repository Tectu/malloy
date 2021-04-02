#include "malloy/listener.hpp"
#include "malloy/http/router.hpp"
#include "../../logger.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

int main(int argc, char* argv[])
{
    // Initialize logger
    auto logger = logging::logger::instance().make_logger("app");

    // Parameters
    const std::string interface             = "127.0.0.1";
    const std::uint16_t port                = 8080;
    const std::filesystem::path doc_root    = "../../../../examples/static_content";

    // Create top-level router
    auto router = std::make_shared<malloy::http::server::router>(logger->clone("router"));
    {
        using namespace malloy::http;

        router->add(method::get, "/", [](const auto& req) {
            response res{ status::ok };
            res.body() = "<html><body><h1>Hello World!</h1><p>some content...</p></body></html>";
            return res;
        });

        // Create nested router 1
        auto nested_router_1 = std::make_shared<malloy::http::server::router>(logger->clone("router 1"));
        nested_router_1->add(method::get, "/", [](const auto& req){
            response resp{ status::ok };
            resp.body() = "router 1 target \"/\"";
            return resp;
        });
        nested_router_1->add(method::get, "/foo", [](const auto& req){
            response resp{ status::ok };
            resp.body() = "router 1 target \"/foo\"";
            return resp;
        });
        nested_router_1->add(method::get, "/foo/bar", [](const auto& req){
            response resp{ status::ok };
            resp.body() = "router 1 target \"/foo/bar\"";
            return resp;
        });
        nested_router_1->add(method::get, "/foo/\\w+", [](const auto& req){
            response resp{ status::ok };
            resp.body() = "router 1 target \"/foo/\\w+\"";
            return resp;
        });
        router->add("/router1", std::move(nested_router_1));

        // Create nested router 2
        auto nested_router_2 = std::make_shared<malloy::http::server::router>(logger->clone("router 2"));
        nested_router_2->add(method::get, "/", [](const auto& req){
            response resp{ status::ok };
            resp.body() = "router 2 target \"/\"";
            return resp;
        });
        nested_router_2->add(method::get, "/foo", [](const auto& req){
            response resp{ status::ok };
            resp.body() = "router 2 target \"/foo\"";
            return resp;
        });
        router->add("/router2", std::move(nested_router_2));
    }

    // The io_context is required for all I/O
    boost::asio::io_context ioc;

    // Create and launch a listening port
    std::make_shared<malloy::server::listener>(
        logger->clone("listener"),
        ioc,
        boost::asio::ip::tcp::endpoint{ boost::asio::ip::make_address(interface), port},
        router,
        std::make_shared<std::filesystem::path>(doc_root)
    )->run();

    logger->info("starting server...");

    // Run the I/O service on one thread
    logger->info("starting server...");
    auto ioc_thread = std::thread([&ioc]{ ioc.run(); });
    ioc.run();

    return EXIT_SUCCESS;
}
