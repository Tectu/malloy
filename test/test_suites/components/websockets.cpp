#include "../../test.hpp"
#include "../../tls_data.hpp"

#include <boost/asio/error.hpp>
#include <malloy/client/controller.hpp>
#include <malloy/core/utils.hpp>
#include <malloy/server/controller.hpp>
#include <malloy/server/routing/router.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/use_future.hpp>


namespace mc = malloy::client;
namespace ms = malloy::server;
using malloy::tests::embedded::tls_cert;
using malloy::tests::embedded::tls_key;
//using boost::asio::awaitable;

namespace
{
    /*inline void roundtrip_coro(
        const uint16_t port,
        std::function<awaitable<void>(malloy::client::controller&)> setup_client,
        std::function<void(malloy::server::controller&)> setup_server)
    {
        namespace mc = malloy::client;
        namespace ms = malloy::server;
        mc::controller c_ctrl;
        ms::controller s_ctrl;

        malloy::controller::config general_cfg;
        general_cfg.num_threads = 2;
        general_cfg.logger = spdlog::default_logger();

        ms::controller::config server_cfg{general_cfg};
        server_cfg.interface = "127.0.0.1";
        server_cfg.port = port;

        mc::controller::config cli_cfg{general_cfg};

        REQUIRE(s_ctrl.init(server_cfg));
        REQUIRE(c_ctrl.init(cli_cfg));

        setup_server(s_ctrl);
        //boost::asio::co_spawn(setup_client(c_ctrl), boost::asio::use_future).get();

        REQUIRE(s_ctrl.start());
        CHECK(c_ctrl.run());
        c_ctrl.stop().get();
    }*/

	constexpr auto server_msg = "Hello from server";
	constexpr auto cli_msg = "Hello from client";

	template<bool BinaryMode>
    void client_ws_handler(
        malloy::error_code ec,
        std::shared_ptr<malloy::client::websocket::connection> conn
    )
    {
        CHECK(!ec);
        REQUIRE(conn);

        conn->set_binary(BinaryMode);

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

    template<bool BinaryMode>
    void server_ws_handler(
        const malloy::http::request<>& req,
        std::shared_ptr<malloy::server::websocket::connection> conn
    )
    {
        conn->set_binary(BinaryMode);

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



    /**
     * Performs a roundtrip.
     *
     * @tparam Secure Whether to use ws:// or wss://
     * @tparam BinaryMode Whether to configure connections to use binary mode.
     * @param port The port to run the test on.
     */
	template<
        bool Secure,
        bool BinaryMode
    >
    void ws_roundtrip(const uint16_t port)
    {
        // Plain
        if constexpr (!Secure)
            malloy::test::roundtrip(
                port,
                [port](auto& c_ctrl) {
                    c_ctrl.ws_connect("127.0.0.1", port, "/", &client_ws_handler<BinaryMode>);
                },
                [](auto& s_ctrl) {
                    s_ctrl.router()->add_websocket("/", &server_ws_handler<BinaryMode>);
                }
            );

        // Secure
        else
            malloy::test::roundtrip(
                port,
                [port](auto& c_ctrl) {
                    REQUIRE(c_ctrl.init_tls());
                    c_ctrl.add_ca(std::string{tls_cert});

                    c_ctrl.wss_connect("127.0.0.1", port, "/", &client_ws_handler<BinaryMode>);
                },
                [](auto& s_ctrl) {
                    REQUIRE(s_ctrl.init_tls(std::string{tls_cert}, std::string{tls_key}));

                    s_ctrl.router()->add_websocket("/", &server_ws_handler<BinaryMode>);
                }
            );
    }

}    // namespace

namespace boost::system {

    auto operator<<(std::ostream& is, const error_condition& cond) -> std::ostream& {
        is << cond.value();
        return is;
    }

}

TEST_SUITE("websockets")
{
    constexpr uint16_t port = 13312;
    constexpr auto loopback = "127.0.0.1";

    TEST_CASE("force_disconnect bypasses all queues")
    {
        constexpr uint16_t lport = 13311;
        std::promise<std::shared_ptr<malloy::client::websocket::connection>> cli_conn_prom;
        auto cli_conn = cli_conn_prom.get_future();
        malloy::test::roundtrip(
            lport, [&](auto& c_ctrl) mutable { c_ctrl.ws_connect(loopback, lport, "/",
                                                                 [&](auto ec, auto conn) mutable {
                                                                     REQUIRE(!ec);
                                                                     auto buff = std::make_shared<boost::beast::flat_buffer>();
                                                                     conn->read(*buff, [buff](malloy::error_code ec, auto) {
                                                                         REQUIRE(ec.value() == boost::asio::error::operation_aborted);
                                                                     });
                                                                     cli_conn_prom.set_value(conn);
                                                                 }); },
            [&cli_conn](auto& s_ctrl) mutable {
                s_ctrl.router()->add_websocket("/",
                                               [&](const auto& req, auto conn) mutable {
                                                   conn->accept(req, [conn, &cli_conn]() mutable {
                                                       auto buff = std::make_shared<boost::beast::flat_buffer>();
                                                       conn->read(*buff, [buff](auto ec, auto) {
                                                           REQUIRE(ec.value() == 1);    // Connection closed gracefully
                                                       });
                                                       cli_conn.get()->force_disconnect();
                                                   });
                                               });
            });
    }

    TEST_CASE("queued send/reads") {
        constexpr uint16_t local_port = 13313;
        constexpr auto bounceback = "Hello";

        malloy::test::roundtrip(local_port, [bounceback](auto& c_ctrl){
            c_ctrl.ws_connect("127.0.0.1", local_port, "/", [bounceback](auto ec, auto conn){
                    REQUIRE(!ec);
                    auto read_buff = std::make_shared<boost::beast::flat_buffer>();
                    conn->read(*read_buff, [bounceback, conn, read_buff](auto ec, auto){
                        REQUIRE(!ec);
                        auto msg = std::make_shared<std::string>(bounceback);
                        conn->send(malloy::buffer(msg->data(), msg->size()), [msg](auto ec, auto){
                            REQUIRE(!ec);
                        });
                    });
            });
        }, [bounceback](auto& s_ctrl){
                s_ctrl.router()->add_websocket("/", [&](const auto& req, auto conn){
                    conn->accept(req, [bounceback, conn]{
                        auto msg = std::make_shared<std::string>(bounceback);
                        conn->send(malloy::buffer(msg->data(), msg->size()), [msg](auto ec, auto){
                            CHECK(!ec);
                        });

                        auto read_buff = std::make_shared<boost::beast::flat_buffer>();
                        conn->read(*read_buff, [msg, read_buff](auto ec, auto){
                            CHECK(!ec);
                            const auto data = boost::beast::buffers_to_string(read_buff->data());
                            CHECK(data == *msg);
                        });
                    });
                });
            });
    }

    TEST_CASE("roundtrips")
	{
	    SUBCASE("plain, text_mode")
	    {
	        ws_roundtrip<false, false>(port);
        }

        SUBCASE("plain, binary_mode")
        {
            ws_roundtrip<false, true>(port);
        }

#if MALLOY_FEATURE_TLS
        SUBCASE("tls, text_mode")
        {
            ws_roundtrip<true, false>(port);
        }

        SUBCASE("tls, binary_mode")
        {
            ws_roundtrip<true, true>(port);
        }
#endif

        SUBCASE("ws_connect coroutines") {
            /*roundtrip_coro(port, [](auto& ctrl) -> awaitable<void>{
                    auto v = co_await ctrl.ws_connect("127.0.0.1", port, "/", boost::asio::use_awaitable);
                    client_ws_handler<false>(malloy::error_code{}, v);
                    }, [](auto& ctrl){
                    ctrl.router()->add_websocket("/", &server_ws_handler<false>);
                    });*/
            malloy::test::roundtrip(port, [](auto& ctrl) {
                    auto v = ctrl.ws_connect("127.0.0.1", port, "/", boost::asio::use_future).get();
                    client_ws_handler<false>(malloy::error_code{}, v);
                    }, [](auto& ctrl){
                    ctrl.router()->add_websocket("/", &server_ws_handler<false>);
                    });
        }
    }
}
