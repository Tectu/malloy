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
            const auto& output = http::split_header_value("foo");
            REQUIRE_EQ(output.size(), 1);
            CHECK_EQ(output[0], "foo");
        }

        SUBCASE("regular 2")
        {
            const auto& output = http::split_header_value("foo; ");
            REQUIRE_EQ(output.size(), 1);
            CHECK_EQ(output[0], "foo");
        }

        SUBCASE("regular 3")
        {
            const std::string input{ "multipart/form-data; boundary=----WebKitFormBoundarynBjZTMv9eqwyCWhj" };

            const auto& output = http::split_header_value(input);
            REQUIRE_EQ(output.size(), 2);
            CHECK_EQ(output[0], "multipart/form-data");
            CHECK_EQ(output[1], "boundary=----WebKitFormBoundarynBjZTMv9eqwyCWhj");
        }
    }
}

TEST_SUITE("components - core - utils - http - cookies")
{

    TEST_CASE("cookie_value()")
    {
        using namespace std::string_view_literals;

        // ToDo: Don't use req.insert() - Use malloy provided infrastructure to set cookies instead.

        malloy::http::request req;

        SUBCASE("none")
        {
            CHECK_EQ(cookie_value(req, "foo"), std::nullopt);
        }

        SUBCASE("one")
        {
            std::string_view cookie_str = "foo=bar"sv;

            SUBCASE("exists")
            {
                req.insert(malloy::http::field::cookie, cookie_str);
                auto o = cookie_value(req, "foo");
                REQUIRE(o.has_value());
                CHECK_EQ(*o, "bar");
            }

            SUBCASE("non-exist")
            {
                auto o = cookie_value(req, "nope");
                CHECK_FALSE(o.has_value());

                req.insert(malloy::http::field::cookie, cookie_str);

                o = cookie_value(req, "nope");
                CHECK_FALSE(o.has_value());
            }
        }

        SUBCASE("multiple")
        {
            std::string_view cookie_str = "foo=bar; one=two; something=else";

            SUBCASE("exists")
            {
                req.insert(malloy::http::field::cookie, cookie_str);
                auto o = cookie_value(req, "foo");
                REQUIRE(o.has_value());
                CHECK_EQ(*o, "bar");
            }

            SUBCASE("non-exist")
            {
                auto o = cookie_value(req, "nope");
                CHECK_FALSE(o.has_value());

                req.insert(malloy::http::field::cookie, cookie_str);

                o = cookie_value(req, "nope");
                CHECK_FALSE(o.has_value());
            }
        }
    }

}
