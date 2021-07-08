#include "../../test.hpp"

#include <malloy/core/http/generator.hpp>

using namespace malloy::http;

TEST_SUITE("components - http - generator")
{

    TEST_CASE("built-in responses")
    {
        SUBCASE("400 - bad request")
        {
            auto r = generator::bad_request("foobar");

            REQUIRE_EQ(r.status(), status::bad_request);
            REQUIRE_EQ(static_cast<int>(r.status()), 400);
            REQUIRE_EQ(r.body(), "foobar");
        }

        SUBCASE("404 - not found")
        {
            auto r = generator::not_found("foobar");

            REQUIRE_EQ(r.status(), status::not_found);
            REQUIRE_EQ(static_cast<int>(r.status()), 404);
        }

        SUBCASE("500 - server error")
        {
            auto r = generator::server_error("foobar");

            REQUIRE_EQ(r.status(), status::internal_server_error);
            REQUIRE_EQ(static_cast<int>(r.status()), 500);
        }
    }

}
