#include "../../example.hpp"

#include <malloy/client/controller.hpp>

#include <iostream>

boost::asio::awaitable<void>
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

    // Make request
    auto resp = co_await c.http_request(
        malloy::http::method::get,
        "http://www.google.com"
    );
    if (!resp)
        spdlog::error("error: {}", resp.error().message());

    std::cout << *resp << std::endl;
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
