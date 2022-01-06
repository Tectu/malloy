#include "../../example.hpp"

#include <malloy/client/controller.hpp>

#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/co_spawn.hpp>

#include <iostream>

auto make_request() -> boost::asio::awaitable<void> {
    // Create the controller config
    malloy::client::controller::config cfg;
    cfg.num_threads = 1;
    cfg.logger      = create_example_logger();

    // Create the controller
    malloy::client::controller c;
    if (!c.init(cfg)) {
        throw std::runtime_error{"initializing controller failed."};
    }

    // Start
    if (!c.start()) {
        throw std::runtime_error{"starting controller failed."};
    }

    malloy::http::request req(
        malloy::http::method::get,
        "www.google.com",
        80,
        "/"
    );
    auto resp = co_await c.http_request(req, boost::asio::use_awaitable);
    std::cout << resp << '\n';

}

int main()
{
    boost::asio::io_context ioc;
    boost::asio::co_spawn(ioc, make_request(), boost::asio::use_future);
    ioc.run();
    return EXIT_SUCCESS;
}
