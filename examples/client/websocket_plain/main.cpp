#include "../../example.hpp"
#include "../../ws_handlers.hpp"

#include <malloy/client/controller.hpp>

#include <iostream>

malloy::awaitable<void>
example()
{
    // Create the controller config
    malloy::client::controller::config cfg;
    cfg.num_threads = 1;
    cfg.logger      = create_example_logger();

    // Create the controller
    malloy::client::controller c{cfg};

    // Start
    [[maybe_unused]] auto session = start(c);

    // Connect to the /echo endpoint of the websocket example server
    {
        auto ec1 = co_await c.ws_connect(
        "ws://127.0.0.1:8080/echo",
        [](malloy::error_code ec, auto conn) {
            // Was the connection attempt successful?
            if (ec) {
                std::cerr << "could not connect: " << ec.message() << '\n';
                return;
            }

            conn->send(malloy::buffer("Hello from Malloy!"), [conn](auto ec, auto) {
                // Was the sending attempt successful?
                if (ec) {
                    std::cerr << "could not send message to server: " << ec.message() << "\n";
                    return;
                }

                // Read
                malloy::examples::ws::oneshot_read(conn, [](malloy::error_code ec, std::string msg) {
                    std::cout << msg << '\n';
                });
            });
        });
    }

    // Connect to the /timer endpoint of the websocket example server
    {
        auto ec2 = co_await c.ws_connect(
        "ws://127.0.0.1:8080/timer",
        [](malloy::error_code ec, auto conn) {
            // Was the connection attempt successful?
            if (ec) {
                std::cerr << "could not connect: " << ec.message() << '\n';
                return;
            }

            // Send something to the server
            conn->send(malloy::buffer("Whoop Whoop"), [conn](auto ec, auto) {
                // Was the sending attempt successful?
                if (ec) {
                    std::cerr << "could not send message to server: " << ec.message() << "\n";
                    return;
                }

                // Read
                malloy::examples::ws::oneshot_read(conn, [](malloy::error_code ec, std::string msg) {
                    std::cout << msg << std::endl;
                });
            });
        });
    }

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(15s);

    // Session will stop the client controller when it goes out of scope

    co_return;
}

// Include main() which will invoke the example() coroutine
#include "../client_example_main.hpp"
