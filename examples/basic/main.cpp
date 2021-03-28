#include "../../lib/malloy/listener.hpp"
#include "../../lib/malloy/http/router.hpp"
#include "../common/logger.hpp"

#include <boost/asio/signal_set.hpp>
#include <spdlog/logger.h>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

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
    auto const address = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const doc_root = std::make_shared<std::filesystem::path>(argv[3]);
    auto const threads = std::max<int>(1, std::atoi(argv[4]));

    // Create request handler
    auto router = std::make_shared<malloy::http::server::router>(logger->clone("router"));
    {
        using namespace malloy::http;

        router->add(method::get, "/", [](const auto& req) {
            response res{http::status::ok};
            res.body() = "<html><body><h1>Hello World!</h1><p>some content...</p></body></html>";
            return res;
        });

        router->add(method::post, "/page_editor", [](const auto& req) {
            std::cout << req.body() << std::endl;

            response res{status::ok};
            return res;
        });

        router->add(method::get, "/page_editor", [](const auto& req) {
            std::cout << req.body() << std::endl;

            response res{status::ok};
            return res;
        });

        router->add(method::get, "/page/.+", [](const auto& req) {
            std::cout << "target   : " << req.target() << std::endl;
            std::cout << "endpoint : " << req.uri().resource() << std::endl;
            std::cout << "query str: " << req.uri().query_string() << std::endl;
            for (const auto& [key, value] : req.uri().query())
                std::cout << key << " = " << value << std::endl;

            response res{ status::ok };
            return res;
        });
    }

    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<malloy::server::listener>(
        logger->clone("listener"),
        ioc,
        tcp::endpoint{address, port},
        router,
        doc_root
    )->run();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
        [&](boost::system::error_code const&, int)
        {
            // Stop the `io_context`. This will cause `run()`
            // to return immediately, eventually destroying the
            // `io_context` and all of the sockets in it.
            ioc.stop();
        }
    );

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

    // If we get here, it means that we got a SIGINT or SIGTERM
    logger->info("received SIGINT or SIGTERM.");

    logger->info("waiting for all i/o threads to finish...");
    for (auto& t : v)
        t.join();

    // Shut down complete.
    logger->info("shutdown complete.");

    return EXIT_SUCCESS;
}
