#include "../../example.hpp"
#include "../../ws_handlers.hpp"

#include <malloy/client/controller.hpp>

#include <iostream>

int main()
{
    // Create the controller config
    malloy::client::controller::config cfg;
    cfg.num_threads = 1;
    cfg.logger      = create_example_logger();

    // Create the controller
    malloy::client::controller c{cfg};

    // Initialize TLS
    if (!c.init_tls()) {
        std::cerr << "initializing TLS context failed." << std::endl;
        return EXIT_FAILURE;
    }
    c.add_ca_file("../../examples/server/static_content/malloy.cert");

    // Start
    [[maybe_unused]] auto session = start(c);

    // Connect to the /echo endpoint of the websocket example server
    c.wss_connect(
        "127.0.0.1",
        8080,
        "/echo",
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

    // Connect to the /timer endpoint of the websocket example server
    c.wss_connect(
        "127.0.0.1",
        8080,
        "/timer",
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
        }
    );

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(15s);

    // Session will stop the client controller when it goes out of scope

    return EXIT_SUCCESS;
}
