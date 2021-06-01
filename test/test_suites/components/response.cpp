#include "../../test.hpp"

#include <malloy/http/response.hpp>

using namespace malloy::http;

TEST_SUITE("components - http - response")
{
    TEST_CASE("construction")
    {
        SUBCASE("default")
        {
            REQUIRE_NOTHROW(response resp);
        }

        SUBCASE("copy")
        {
            response resp;
        }
    }

    TEST_CASE("operators")
    {
        response lhs;
        response rhs;

        SUBCASE("copy assignment")
        {
            REQUIRE_NOTHROW(rhs = lhs);
        }

        SUBCASE("move assignment")
        {
            REQUIRE_NOTHROW(rhs = std::move(lhs));
        }
    }
}
