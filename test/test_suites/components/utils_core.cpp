#include "../../test.hpp"

#include <malloy/core/utils.hpp>

TEST_SUITE("components - utils - core")
{
    using namespace malloy;
    using namespace std::literals;

    TEST_CASE("string splitting")
    {
        SUBCASE("empty string")
        {
            const auto& out = split(""sv, ","sv);
            REQUIRE_EQ(out.size(), 0);
        }

        SUBCASE("empty delimiter")
        {
            const std::string_view in{ "hello,world,test!"sv };
            const auto& out = split(in, ""sv);
            REQUIRE_EQ(out.size(), 1);
            CHECK_EQ(out[0], in);
        }

        SUBCASE("empty string and empty delimiter")
        {
            const auto& out = split(""sv, ""sv);
            REQUIRE_EQ(out.size(), 0);
        }

        SUBCASE("one-char delimiter")
        {
            const std::string_view in{ "hello,world,test!"sv };
            const auto& out = split(in, ",");
            REQUIRE_EQ(out.size(), 3);
            CHECK_EQ(out[0], "hello");
            CHECK_EQ(out[1], "world");
            CHECK_EQ(out[2], "test!");
        }

        SUBCASE("multi-char delimiter")
        {
            const std::string_view in{ "hello,;,world,;,test!"sv };
            const auto& out = split(in, ",;,");
            REQUIRE_EQ(out.size(), 3);
            CHECK_EQ(out[0], "hello");
            CHECK_EQ(out[1], "world");
            CHECK_EQ(out[2], "test!");

        }

        SUBCASE("leading delimiter")
        {
            const std::string_view in{ ",hello,world,test!"sv };
            const auto& out = split(in, ","sv);
            REQUIRE_EQ(out.size(), 3);
            CHECK_EQ(out[0], "hello");
            CHECK_EQ(out[1], "world");
            CHECK_EQ(out[2], "test!");
        }

        SUBCASE("trailing delimiter")
        {
            const std::string_view in{ "hello,world,test!,"sv };
            const auto& out = split(in, ","sv);
            REQUIRE_EQ(out.size(), 3);
            CHECK_EQ(out[0], "hello");
            CHECK_EQ(out[1], "world");
            CHECK_EQ(out[2], "test!");
        }
    }

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
