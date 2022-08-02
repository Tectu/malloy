#include "../../test.hpp"

#include <malloy/server/routing/router.hpp>
#include <malloy/server/routing_context.hpp>

using namespace malloy::http;
using namespace malloy::server;

TEST_SUITE("components - router")
{
    TEST_CASE("policies block unaccepted requests") {
        constexpr auto port = 44173;
        malloy::test::roundtrip(port, [&, port](auto& c_ctrl){
                request<> req{method::get, "127.0.0.1", port, "/"};
                auto st = c_ctrl.http_request(req, [](const auto& resp){
                    CHECK(resp.result() == status::unauthorized);
                });

        }, [&](auto& s_ctrl){
                auto& r = s_ctrl.router();
                r.add(method::get, "/", [](const auto& req){
                    return generator::ok();
                });
                r.add_policy("/", [](auto) -> std::optional<response<>> { return response<>{status::unauthorized}; });
            });
    }

    TEST_CASE("add [regex]") {
        router r;
        SUBCASE("Adding a handler with only a request compiles") {
            r.add(method::get, "", [](const auto&) { return generator::ok(); });
        }
        SUBCASE("Adding a handler with a request and capture results compiles") {
            r.add(method::get, "", [](const auto&, const auto&){ return generator::ok(); });
        }
    }

    TEST_CASE("add [redirect]")
    {
        router r;

        SUBCASE("valid [permanent]")
        {
            CHECK(r.add_redirect(status::permanent_redirect, "/foo", "/bar"));
        }

        SUBCASE("valid [temporary]")
        {
            CHECK(r.add_redirect(status::temporary_redirect, "/foo", "/bar"));
        }

        SUBCASE("invalid [status code]")
        {
            CHECK_FALSE(r.add_redirect(status::ok, "/foo", "/bar"));
        }

        SUBCASE("invalid [resource old]")
        {
            SUBCASE("empty")
            {
                CHECK_FALSE(r.add_redirect(status::permanent_redirect, "", "/bar"));
            }

            SUBCASE("no leading slash")
            {
                CHECK_FALSE(r.add_redirect(status::permanent_redirect, "foo", "/bar"));
            }
        }

        SUBCASE("invalid [resource new]")
        {
            SUBCASE("empty")
            {
                CHECK_FALSE(r.add_redirect(status::permanent_redirect, "/foo", ""));
            }

            SUBCASE("no leading slash")
            {
                CHECK_FALSE(r.add_redirect(status::permanent_redirect, "/foo", "bar"));
            }
        }
        
    }
    TEST_CASE("server_string propagation") {
        constexpr auto server_str = "hello";
        routing_context::config cfg;
        cfg.logger = spdlog::default_logger();
        cfg.connection_logger = spdlog::default_logger();
        cfg.agent_string = server_str;
        routing_context ctrl{cfg};

        SUBCASE("server string propagates to child routers") {
            constexpr auto server_str2 = "hello2";
            router r1{nullptr};
            CHECK(r1.server_string().empty());

            auto sub1_sm = std::make_unique<router>();
            auto* sub1 = sub1_sm.get();
            CHECK(sub1->server_string().empty());
            r1.add_subrouter("/", std::move(sub1_sm));

            CHECK(ctrl.router().server_string() == server_str);
            ctrl.router().add_subrouter("/", std::move(r1));

            CHECK(sub1->server_string() == server_str);

        }
        SUBCASE("server string propagation composes over multiple levels") {
            auto sub1_sm = std::make_unique<router>();
            auto sub1_1_sm = std::make_unique<router>();
            auto* sub1 = sub1_sm.get();
            auto* sub1_1 = sub1_1_sm.get();

            sub1->add_subrouter("/", std::move(sub1_1_sm));
            REQUIRE(sub1_1->server_string().empty());
            REQUIRE(sub1->server_string().empty());

            ctrl.router().add_subrouter("/", std::move(sub1_sm));
            REQUIRE(sub1_1->server_string() == server_str);
            REQUIRE(sub1->server_string() == server_str);
        }
    }

}
