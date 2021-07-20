#include "../../test.hpp"

#include <malloy/core/http/utils.hpp>

TEST_SUITE("components - utils - core - http")
{
    using namespace malloy;

    TEST_CASE("header value splitting")
    {
        SUBCASE("empty")
        {
            const auto& output = http::split_header_value("");
            REQUIRE_EQ(output.size(), 0);
        }

        SUBCASE("regular 1")
        {
            const std::string input{ "multipart/form-data; boundary=----WebKitFormBoundarynBjZTMv9eqwyCWhj" };

            const auto& output = http::split_header_value(input);
            REQUIRE_EQ(output.size(), 2);
            CHECK_EQ(output[0], "multipart/form-data");
            CHECK_EQ(output[1], "boundary=----WebKitFormBoundarynBjZTMv9eqwyCWhj");
        }
    }
}
