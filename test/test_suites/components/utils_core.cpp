#include "../../test.hpp"

#include <malloy/core/utils.hpp>

TEST_SUITE("components - utils - core")
{
    using namespace malloy;

    TEST_CASE("URL decoding")
    {
        /**
         * Performs URL decoding and checks against expected output.
         */
        auto test = [](std::string&& input, const std::string& expected_output)
        {
            REQUIRE_NOTHROW(url_decode(input));
            CHECK_EQ(input, expected_output);
        };

        SUBCASE("empty")
        {
            test("", "");
        }

        SUBCASE("no replacement needed")
        {
            test("nothing to do here", "nothing to do here");
        }

        SUBCASE("regular")
        {
            test("some%25thi%3Eng%20here%3f", "some%thi>ng here?");
        }
    }
}
