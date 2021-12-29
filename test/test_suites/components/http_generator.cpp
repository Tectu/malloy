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

            CHECK_EQ(r.status(), status::bad_request);
            CHECK_EQ(static_cast<int>(r.status()), 400);
            CHECK_EQ(r.body(), "foobar");
        }

        SUBCASE("404 - not found")
        {
            auto r = generator::not_found("foobar");

            CHECK_EQ(r.status(), status::not_found);
            CHECK_EQ(static_cast<int>(r.status()), 404);
            CHECK_EQ(r.body(), "The resource 'foobar' was not found.");
        }

        SUBCASE("500 - server error")
        {
            auto r = generator::server_error("foobar");

            CHECK_EQ(r.status(), status::internal_server_error);
            CHECK_EQ(static_cast<int>(r.status()), 500);
            CHECK_EQ(r.body(), "An error occurred: 'foobar'");
        }
    }

}
