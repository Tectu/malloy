#include "../../example.hpp"
#include "../../ws_handlers.hpp"

#include <malloy/client/controller.hpp>

#include <print>

malloy::awaitable<void>
example()
{
    // Create the controller config
    malloy::client::controller::config cfg;
    cfg.num_threads = 1;
    cfg.logger      = create_example_logger();

    // Create the controller
    malloy::client::controller c{cfg};

#if MALLOY_FEATURE_TLS
    // Initialize TLS context
    if (!c.init_tls()) {
        spdlog::error("initializing TLS context failed.");
        co_return;
    }
#endif

    // Start
    [[maybe_unused]] auto session = start(c);

    // Connect to the /echo endpoint of the websocket example server
    {
        // Make connection
        auto conn_exp = co_await c.ws_connect("ws://127.0.0.1:8080/echo");
        //auto conn_exp = co_await c.ws_connect("wss://echo.websocket.org/");
        if (!conn_exp) {
            spdlog::error("could not make websocket connection: {}", conn_exp.error().message());
            co_return;
        }
        auto conn = std::move(conn_exp.value());

        // Send message
        const auto bytes_written = co_await conn->send("Hello from Malloy!");
        if (!bytes_written)
            spdlog::error("could not to send data to websocket server: {}", bytes_written.error().message());

        // Read message (read echo)
        const auto read = co_await conn->read();
        if (!read)
            spdlog::error("could not read data from websocket: {}", read.error().message());

        // Print
        std::println("{}", malloy::buffers_to_string(malloy::buffer(read->cdata(), read->size())));
    }

    // Connect to the /timer endpoint of the websocket example server
    {
        // Make connection
        auto conn_exp = co_await c.ws_connect("ws://127.0.0.1:8080/timer");
        if (!conn_exp) {
            spdlog::error("could not make websocket connection: {}", conn_exp.error().message());
            co_return;
        }
        auto conn = std::move(conn_exp.value());

        // Send something to the server
        co_await conn->send("");

        // Read until the connection was closed (by the server)
        while (true) {
            const auto read = co_await conn->read();
            if (!read) {
                const auto ec = read.error();
                if (ec == malloy::websocket::error::closed)
                    spdlog::info("server closed connection");
                else
                    spdlog::error("could not read data from websocket: {}", ec.message());

                break;
            } else
                // Print
                std::println("{}", malloy::buffers_to_string(malloy::buffer(read->cdata(), read->size())));
        }
    }

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(15s);

    // Session will stop the client controller when it goes out of scope

    co_return;
}

// Include main() which will invoke the example() coroutine
#include "../client_example_main.hpp"
