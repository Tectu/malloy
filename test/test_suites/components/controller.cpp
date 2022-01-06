#include "../../test.hpp"

#include <malloy/server/controller.hpp>
#include <malloy/server/routing/router.hpp>
#include <malloy/client/controller.hpp>

namespace mc = malloy::client;
namespace ms = malloy::server;

TEST_SUITE("controller - roundtrips") {
    TEST_CASE("Server and client set agent strings based on user_agent") {
        constexpr auto cli_agent_str = "test-cli";
        constexpr auto serve_agent_str = "test-serve";
        constexpr auto addr = "127.0.0.1";
        constexpr uint16_t port = 55123;


        malloy::controller::config general_cfg;
        general_cfg.logger = spdlog::default_logger();

        mc::controller::config cli_cfg{general_cfg};
        ms::controller::config serve_cfg{general_cfg};

        cli_cfg.user_agent = cli_agent_str;
        serve_cfg.agent_string = serve_agent_str;
        serve_cfg.interface = addr;
        serve_cfg.port = port;

        mc::controller cli_ctrl;

        REQUIRE(cli_ctrl.init(cli_cfg));

        malloy::http::request<> req{
            malloy::http::method::get,
            addr,
            port,
            "/"
        };
        auto stop_tkn = cli_ctrl.http_request(req, [&](auto&& resp){
            CHECK(resp[malloy::http::field::server] == serve_agent_str);
        });

        ms::controller serve_ctrl{serve_cfg};


        serve_ctrl.router().add(malloy::http::method::get, "/", [&](auto&& req){
            CHECK(req[malloy::http::field::user_agent] == cli_agent_str);
            return malloy::http::generator::ok();
        });

        auto serve_session = start(std::move(serve_ctrl));
        REQUIRE(cli_ctrl.run());

        CHECK(!stop_tkn.get());

    }
}

