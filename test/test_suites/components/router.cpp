#include "../../test.hpp"

#include <malloy/server/routing/router.hpp>

using namespace malloy::http;
using namespace malloy::server;

TEST_SUITE("components - router")
{

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
    TEST_CASE("A ")

}
