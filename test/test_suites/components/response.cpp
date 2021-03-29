#include "../../test.hpp"

#include <malloy/http/response.hpp>

using namespace malloy::http;

TEST_SUITE("components - response")
{

    TEST_CASE("built-in responses")
    {
        SUBCASE("400 - bad request")
        {
            auto r = response::bad_request("foobar");

            REQUIRE_EQ(r.status(), status::bad_request);
            REQUIRE_EQ(r.body(), "foobar");
        }

        SUBCASE("404 - not found")
        {
            auto r = response::not_found("foobar");

            REQUIRE_EQ(r.status(), status::not_found);
        }

        SUBCASE("500 - server error")
        {
            auto r = response::server_error("foobar");

            REQUIRE_EQ(r.status(), status::internal_server_error);
        }
    }

}
