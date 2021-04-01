#include "../../test.hpp"

#include <malloy/http/uri.hpp>

using namespace malloy;
using namespace malloy::http;

TEST_SUITE("components - uri")
{

    TEST_CASE("empty")
    {
        uri u;

        REQUIRE_NOTHROW(u = uri{""});

        REQUIRE_EQ(u.resource_string(), "");
        REQUIRE_EQ(u.resource().size(), 0);

        REQUIRE_EQ(u.query_string(), "");
        REQUIRE_EQ(u.query().size(), 0);

        REQUIRE_EQ(u.fragment(), "");
    }

    TEST_CASE("/")
    {
        uri u{ "/" };

        REQUIRE_EQ(u.resource_string(), "/");
        REQUIRE_EQ(u.resource().size(), 0);

        REQUIRE_EQ(u.query_string(), "");
        REQUIRE_EQ(u.query().size(), 0);

        REQUIRE_EQ(u.fragment(), "");
    }

    TEST_CASE("compound resource, no query, no fragment")
    {
        uri u{ "/foo/bar" };

        REQUIRE_EQ(u.resource_string(), "/foo/bar");
        REQUIRE_EQ(u.resource().size(), 2);
        CHECK_EQ(u.resource().at(0), "foo");
        CHECK_EQ(u.resource().at(1), "bar");

        REQUIRE_EQ(u.query_string(), "");
        REQUIRE_EQ(u.query().size(), 0);

        REQUIRE_EQ(u.fragment(), "");
    }

    TEST_CASE("compound resource, single query, no fragment")
    {
        uri u{ "/foo/bar?test1=1" };

        REQUIRE_EQ(u.resource_string(), "/foo/bar");
        REQUIRE_EQ(u.resource().size(), 2);
        CHECK_EQ(u.resource().at(0), "foo");
        CHECK_EQ(u.resource().at(1), "bar");

        REQUIRE_EQ(u.query_string(), "test1=1");
        REQUIRE_EQ(u.query().size(), 1);
        CHECK_EQ(u.query().at("test1"), "1");

        REQUIRE_EQ(u.fragment(), "");
    }

    TEST_CASE("compound resource, multiple queries, no fragment")
    {
        uri u{ "/foo/bar?test1=1&test2=2&test3=3" };

        REQUIRE_EQ(u.resource_string(), "/foo/bar");
        REQUIRE_EQ(u.resource().size(), 2);
        CHECK_EQ(u.resource().at(0), "foo");
        CHECK_EQ(u.resource().at(1), "bar");

        REQUIRE_EQ(u.query_string(), "test1=1&test2=2&test3=3");
        REQUIRE_EQ(u.query().size(), 3);
        CHECK_EQ(u.query().at("test1"), "1");
        CHECK_EQ(u.query().at("test2"), "2");
        CHECK_EQ(u.query().at("test3"), "3");

        REQUIRE_EQ(u.fragment(), "");
    }

    TEST_CASE("resource chopping")
    {
        SUBCASE("")
        {
            uri u{"/foo/bar/zbar"};

            REQUIRE(u.chop_resource("/foo"));

            REQUIRE_EQ(u.resource_string(), "/bar/zbar");
            REQUIRE_EQ(u.resource().size(), 2);
            REQUIRE_EQ(u.resource().at(0), "bar");
            REQUIRE_EQ(u.resource().at(1), "zbar");

            REQUIRE(u.resource_starts_with("/bar"));
        }

        SUBCASE("")
        {
            uri u{"/foo/bar/zbar"};

            REQUIRE(u.chop_resource("/foo/bar"));

            REQUIRE_EQ(u.resource_string(), "/zbar");
            REQUIRE_EQ(u.resource().size(), 1);
            REQUIRE_EQ(u.resource().at(0), "zbar");

            REQUIRE(u.resource_starts_with("/zbar"));
        }
    }

    TEST_CASE("legality")
    {

        SUBCASE("empty")
        {
            REQUIRE_FALSE(uri{""}.is_legal());
        }

        SUBCASE("not starting with '/'")
        {
            REQUIRE_FALSE(uri{"foo/bar"}.is_legal());
        }

        SUBCASE("contains \"...\"")
        {
            REQUIRE_FALSE(uri{"/foo/../bar"}.is_legal());
            REQUIRE_FALSE(uri{"/.."}.is_legal());
            REQUIRE_FALSE(uri{"/../../../foo.html"}.is_legal());
        }

    }

}
