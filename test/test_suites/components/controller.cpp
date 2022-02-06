#include "../../test.hpp"

#include <malloy/server/routing_context.hpp>
#include <malloy/server/routing/router.hpp>
#include <malloy/client/controller.hpp>

namespace mc = malloy::client;
namespace ms = malloy::server;

template<typename T>
concept check_start_result = requires(T v) {
    { start(std::forward<T>(v)) } -> std::same_as<typename std::remove_cvref_t<T>::session>;
};

template<typename T>
concept check_start_accessible = requires(mc::controller& v) {
    { mc::start(v) };
} && requires(ms::routing_context&& v) {
    { ms::start(std::move(v)) };
};

static_assert(check_start_accessible<std::void_t<>>);
static_assert(check_start_result<mc::controller&>, "start returns the type of start_result for client controller");
static_assert(check_start_result<ms::routing_context>, "start returns the type of start_result for server controller");

TEST_SUITE("controller - compile checks") {
    TEST_CASE("start(...) is accessible directly from the parent namespace of controller/routing context"){
    }
}
TEST_SUITE("controller - roundtrips") {
    TEST_CASE("A controller_run_result<T> where T is moveable is also movable and well defined") {
        mc::controller::config cfg;
        cfg.logger = spdlog::default_logger();
        cfg.num_threads = 1;
        mc::controller ctrl{cfg};

        auto tkn = start(ctrl);
        auto tkn2 = std::move(tkn);
        SUBCASE("calling run on a moved-from run result raises an exception") {
            CHECK_THROWS(tkn.run());
        }
        SUBCASE("calling run on the move constructed result is well defined") {
            tkn2.run();
        }
    }
    TEST_CASE("Server and client set agent strings based on user_agent") {
        constexpr auto cli_agent_str = "test-cli";
        constexpr auto serve_agent_str = "test-serve";
        constexpr auto addr = "127.0.0.1";
        constexpr uint16_t port = 55123;


        malloy::controller::config general_cfg;
        general_cfg.logger = spdlog::default_logger();

        mc::controller::config cli_cfg{general_cfg};
        ms::routing_context::config serve_cfg{general_cfg};

        cli_cfg.user_agent = cli_agent_str;
        serve_cfg.agent_string = serve_agent_str;
        serve_cfg.interface = addr;
        serve_cfg.port = port;

        mc::controller cli_ctrl{cli_cfg};

        malloy::http::request<> req{
            malloy::http::method::get,
            addr,
            port,
            "/"
        };
        auto stop_tkn = cli_ctrl.http_request(req, [&](auto&& resp){
            CHECK(resp[malloy::http::field::server] == serve_agent_str);
        });

        ms::routing_context serve_ctrl{serve_cfg};


        serve_ctrl.router().add(malloy::http::method::get, "/", [&](auto&& req){
            CHECK(req[malloy::http::field::user_agent] == cli_agent_str);
            return malloy::http::generator::ok();
        });

        auto serve_session = start(std::move(serve_ctrl));
        start(cli_ctrl).run();

        CHECK(!stop_tkn.get());

    }
}

