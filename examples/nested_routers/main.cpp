#include "../../lib/malloy/listener.hpp"
#include "../../lib/malloy/http/router.hpp"
#include "../common/logger.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

int main(int argc, char* argv[])
{
    // Initialize logger
    std::shared_ptr<spdlog::logger> logger;
    try {
        logger = logging::logger::instance().make_logger("app");
    }
    catch (const std::exception& e) {
        std::cerr << "could not create logger: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "could not create logger." << std::endl;
        return EXIT_FAILURE;
    }

    // Check command line arguments.
    if (argc != 5) {
        std::cerr <<
            "Usage: malloy <address> <port> <doc_root> <threads>\n" <<
            "Example:\n" <<
            "    malloy 0.0.0.0 8080 . 1\n";
        return EXIT_FAILURE;
    }
    auto const address = boost::asio::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const doc_root = std::make_shared<std::filesystem::path>(argv[3]);
    auto const threads = std::max<int>(1, std::atoi(argv[4]));

    // Create top-level router
    auto router = std::make_shared<malloy::http::server::router>(logger->clone("router"));
    {
        using namespace malloy::http;

        router->add(method::get, "/", [](const auto& req) {
            response res{ http::status::ok };
            res.body() = "<html><body><h1>Hello World!</h1><p>some content...</p></body></html>";
            return res;
        });

        // Create nested router 1
        auto nested_router_1 = std::make_shared<malloy::http::server::router>(logger->clone("router 1"));
        nested_router_1->add(method::get, "/", [](const auto& req){
            response resp{ http::status::ok };
            resp.body() = "router 1 target \"/\"";
            return resp;
        });
        nested_router_1->add(method::get, "/foo", [](const auto& req){
            response resp{ http::status::ok };
            resp.body() = "router 1 target \"/foo\"";
            return resp;
        });
        router->add("/router1", std::move(nested_router_1));

        // Create nested router 2
        auto nested_router_2 = std::make_shared<malloy::http::server::router>(logger->clone("router 2"));
        nested_router_2->add(method::get, "/", [](const auto& req){
            response resp{ http::status::ok };
            resp.body() = "router 2 target \"/\"";
            return resp;
        });
        nested_router_2->add(method::get, "/foo", [](const auto& req){
            response resp{ http::status::ok };
            resp.body() = "router 2 target \"/foo\"";
            return resp;
        });
        router->add("/router2", std::move(nested_router_2));
    }

    // The io_context is required for all I/O
    boost::asio::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<malloy::server::listener>(
        logger->clone("listener"),
        ioc,
        boost::asio::ip::tcp::endpoint{address, port},
        router,
        doc_root
    )->run();

    logger->info("starting server...");

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
            [&ioc]
            {
                ioc.run();
            });
    ioc.run();

    return EXIT_SUCCESS;
}
