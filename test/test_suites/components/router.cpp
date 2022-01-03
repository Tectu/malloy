#include "../../test.hpp"

#include <malloy/server/routing/router.hpp>

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
                auto r = s_ctrl.router();
                r->add(method::get, "/", [](const auto& req){
                    return generator::ok();
                });
                r->add_policy("/", [](auto) -> std::optional<response<>> { return response<>{status::unauthorized}; });
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
        SUBCASE("server string propagates to subrouters without explicit strings") {
            constexpr auto server_str = "hello";
            constexpr auto server_str2 = "hello2";

            router r1{nullptr, server_str};
            CHECK(r1.server_string().has_value());

            auto sub1 = std::make_shared<router>();
            CHECK(!sub1->server_string().has_value());
            r1.add_subrouter("/", sub1);

            REQUIRE(sub1->server_string().has_value());
            CHECK(*sub1->server_string() == server_str);

            auto sub2 = std::make_shared<router>(nullptr, server_str2);
            REQUIRE(sub2->server_string().has_value());
            CHECK(*sub2->server_string() == server_str2);

            r1.add_subrouter("/hello", sub2);
            REQUIRE(sub2->server_string().has_value());
            CHECK(*sub2->server_string() == server_str2);

        }
        SUBCASE("server string propagation composes over multiple levels") {
            constexpr auto server_str = "hello";
            router r1{nullptr, server_str};

            auto sub1 = std::make_shared<router>();
            auto sub1_1 = std::make_shared<router>();

            sub1->add_subrouter("/", sub1_1);
            REQUIRE(!sub1_1->server_string().has_value());
            REQUIRE(!sub1->server_string().has_value());

            r1.add_subrouter("/", sub1);
            REQUIRE(sub1_1->server_string().has_value());
            REQUIRE(sub1->server_string().has_value());
        }

    }

}
