#include "../../example_logger.hpp"
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
    malloy::client::controller c;
    if (!c.init(cfg)) {
        std::cerr << "initializing controller failed." << std::endl;
        return EXIT_FAILURE;
    }

    // Start
    if (!c.start()) {
        std::cerr << "starting controller failed." << std::endl;
        return EXIT_FAILURE;
    }
    if (!c.init_tls()) {
        std::cerr << "initializing TLS context failed." << std::endl;
        return EXIT_FAILURE;
    }
    c.load_ca_file("../../../../examples/server/static_content/malloy.cert");

    c.wss_connect(
        "127.0.0.1",
        8080,
        "/echo",
        [](malloy::error_code ec, auto conn) {
            if (ec) {
                std::cerr << "Couldn't connect: " << ec.message() << '\n';
                return;
            }
            conn->send(malloy::buffer("Hello from Malloy!"), [conn](auto ec, auto) {
                if (ec) {
                    std::cerr << "Uh oh, we couldn't send something: " << ec.message();
                    return;
                }
                malloy::examples::ws::oneshot_read(conn, [](malloy::error_code ec, std::string msg) {
                    std::cout << "id[0]: " << msg << '\n';
                    });
                });
        });

    c.wss_connect(
        "127.0.0.1",
        8080,
        "/timer",
        [](malloy::error_code ec, auto conn) {
            if (ec) {
                std::cerr << "Uh oh, we have a problem: " << ec.message() << '\n';
                return;
            }
            conn->send(malloy::buffer("Whoop Whoop"), [conn](auto ec, auto) {
                malloy::examples::ws::oneshot_read(conn, [](malloy::error_code ec, std::string msg) {
                    std::cout << "id[1]: " << msg << std::endl;
                    });
                });
        }
    );

    c.run();
    c.stop();

    return EXIT_SUCCESS;
}
