#include "../../example.hpp"

#include <malloy/core/http/filters/file.hpp>
#include <malloy/client/controller.hpp>

#include <iostream>

void
log_error(malloy::error_code ec)
{
    if (ec)
        spdlog::error(ec.message());
}

malloy::awaitable<void>
example()
{
    // Create the controller config
    malloy::client::controller::config cfg;
    cfg.logger      = create_example_logger();
    cfg.num_threads = 1;

    // Create the controller
    malloy::client::controller c{cfg};

    // Start
    [[maybe_unused]] auto session = start(c);

    // Make request (with file response filter)
    auto resp = co_await c.http_request(
        "http://www.google.com",
        malloy::http::filters::file_response::open("./google.com.html", log_error, boost::beast::file_mode::write)
    );
    if (!resp)
        spdlog::error("error: {}", resp.error().message());
}

int main()
{
    boost::asio::io_context ioc;

    boost::asio::co_spawn(
        ioc,
        example(),
        boost::asio::use_future
    );

    ioc.run();
}
