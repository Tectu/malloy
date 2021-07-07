#include "../../test.hpp"
#include "../../tls_data.hpp"

#include <malloy/client/controller.hpp>
#include <malloy/server/controller.hpp>
#include <malloy/utils.hpp>
#include <malloy/server/routing/router.hpp>

namespace mc = malloy::client;
namespace ms = malloy::server;
using malloy::tests::embedded::tls_cert;
using malloy::tests::embedded::tls_key;

namespace
{

	constexpr auto server_msg = "Hello from server";
	constexpr auto cli_msg = "Hello from client";

    void client_ws_handler(malloy::error_code ec, std::shared_ptr<malloy::client::websocket::connection> conn)
    {
        REQUIRE(!ec);
        CHECK(conn);

        auto buffer = std::make_shared<boost::beast::flat_buffer>();
        conn->read(*buffer, [&, conn, buffer](auto ec, auto size) {
            CHECK(!ec);
            CHECK(size == std::strlen(server_msg));

            CHECK(malloy::buffers_to_string(buffer->cdata()) == server_msg);

            conn->send(malloy::buffer(cli_msg, std::strlen(cli_msg)), [conn](auto ec, auto size) {
                CHECK(!ec);
                CHECK(size == std::strlen(cli_msg));
            });
        });
    }

    void server_ws_handler(const malloy::http::request<>& req, std::shared_ptr<malloy::server::websocket::connection> conn)
    {
        conn->accept(req, [conn]() mutable {
            conn->send(malloy::buffer(server_msg, std::strlen(server_msg)), [conn](auto ec, auto size) mutable {
                CHECK(!ec);
                CHECK(size == std::strlen(server_msg));
                auto buffer = std::make_shared<boost::beast::flat_buffer>();
                conn->read(*buffer, [buffer](auto ec, auto size) mutable {
                    CHECK(!ec);
                    CHECK(size == std::strlen(cli_msg));

                    CHECK(malloy::buffers_to_string(buffer->cdata()) == cli_msg);
                });
            });
        });
    }

    void ws_roundtrip(
        uint16_t port,
        std::function<void(malloy::client::controller&)> setup_client,
        std::function<void(malloy::server::controller&)> setup_server
    )
    {
        mc::controller c_ctrl;
		ms::controller s_ctrl;

		malloy::controller::config general_cfg;
		general_cfg.num_threads = 2;
		general_cfg.logger = spdlog::default_logger();

		ms::controller::config server_cfg{ general_cfg };
		server_cfg.interface = "127.0.0.1";
		server_cfg.port = port;

		REQUIRE(s_ctrl.init(server_cfg));
		REQUIRE(c_ctrl.init(general_cfg));

        setup_server(s_ctrl);
        setup_client(c_ctrl);

		REQUIRE(s_ctrl.start());
        CHECK(c_ctrl.run());
        c_ctrl.stop().get();
    }

}    // namespace

TEST_SUITE("websockets")
{

	TEST_CASE("roundtrip")
	{
        constexpr uint16_t port = 13312;

        ws_roundtrip(
            port,
            [](auto& c_ctrl) {
                c_ctrl.ws_connect("127.0.0.1", port, "/", &client_ws_handler);
            },
            [](auto& s_ctrl) {
                s_ctrl.router()->add_websocket("/", &server_ws_handler);
            }
        );
    }

#if MALLOY_FEATURE_TLS 
    TEST_CASE("roundtrip - tls")
    {
        constexpr uint16_t port = 13313;

        ws_roundtrip(
            port,
            [](auto& c_ctrl) {
                REQUIRE(c_ctrl.init_tls());
                c_ctrl.add_ca(std::string{tls_cert});

                c_ctrl.wss_connect("127.0.0.1", port, "/", &client_ws_handler);
            },
            [](auto& s_ctrl) {
                REQUIRE(s_ctrl.init_tls(std::string{tls_cert}, std::string{tls_key}));

                s_ctrl.router()->add_websocket("/", &server_ws_handler);
            }
        );
    }
#endif

}
