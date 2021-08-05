#include "../../test.hpp"

#include <malloy/server/routing/router.hpp>

using namespace malloy::http;
using namespace malloy::server;

TEST_SUITE("components - router")
{
    TEST_CASE("policies block unaccepted requests") {
        struct always_blocked {
            auto operator()(auto) -> std::optional<response<>> {
                response<> res{status::unauthorized};
                return res;
            }
        };
        constexpr auto port = 4413;
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
                r->add_policy("/", always_blocked{});
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

}
