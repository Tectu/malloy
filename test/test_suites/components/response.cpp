#include "../../test.hpp"

#include <malloy/http/response.hpp>

using namespace malloy::http;

TEST_SUITE("components - response")
{

    TEST_CASE("")
    {
        auto r = response::bad_request("foobar");

        REQUIRE_EQ(r.status(), status::bad_request);
        REQUIRE_EQ(r.body(), "foobar");
    }

}
