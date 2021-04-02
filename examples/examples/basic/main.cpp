#include "malloy/listener.hpp"
#include "malloy/http/generator.hpp"
#include "malloy/http/router.hpp"
#include "../../logger.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

int main()
{
    // Initialize logger
    auto logger = logging::logger::instance().make_logger("app");

    // Parameters
    const std::string interface             = "127.0.0.1";
    const std::uint16_t port                = 8080;
    const std::filesystem::path doc_root    = "../../../../examples/static_content";

    // Create the router
    auto router = std::make_shared<malloy::http::server::router>(logger->clone("router"));
    {
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
    }

    // The io_context is required for all I/O
    boost::asio::io_context ioc;

    // Create and launch a listener
    std::make_shared<malloy::server::listener>(
        logger->clone("listener"),
        ioc,
        boost::asio::ip::tcp::endpoint{ boost::asio::ip::make_address(interface), port },
        router,
        std::make_shared<std::filesystem::path>(doc_root)
    )->run();

    // Run the I/O service on one thread
    logger->info("starting server...");
    auto ioc_thread = std::thread([&ioc]{ ioc.run(); });
    ioc.run();

    return EXIT_SUCCESS;
}
