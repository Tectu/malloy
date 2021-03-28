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
        REQUIRE_EQ(u.resource(), "");
        REQUIRE_EQ(u.query_string(), "");
        REQUIRE_EQ(u.query().size(), 0);
        REQUIRE_EQ(u.fragment(), "");
    }

    TEST_CASE("compound resource, no query, no fragment")
    {
        uri u{ "/foo/bar" };

        REQUIRE_EQ(u.resource(), "/foo/bar");
        REQUIRE_EQ(u.query_string(), "");
        REQUIRE_EQ(u.query().size(), 0);
        REQUIRE_EQ(u.fragment(), "");
    }

    TEST_CASE("compound resource, single query, no fragment")
    {
        uri u{ "/foo/bar?test1=1" };

        REQUIRE_EQ(u.resource(), "/foo/bar");
        REQUIRE_EQ(u.query_string(), "test1=1");
        REQUIRE_EQ(u.query().size(), 1);

        CHECK_EQ(u.query().at("test1"), "1");

        REQUIRE_EQ(u.fragment(), "");
    }

    TEST_CASE("compound resource, multiple queries, no fragment")
    {
        uri u{ "/foo/bar?test1=1&test2=2&test3=3" };

        REQUIRE_EQ(u.resource(), "/foo/bar");
        REQUIRE_EQ(u.query_string(), "test1=1&test2=2&test3=3");
        REQUIRE_EQ(u.query().size(), 3);

        CHECK_EQ(u.query().at("test1"), "1");
        CHECK_EQ(u.query().at("test2"), "2");
        CHECK_EQ(u.query().at("test3"), "3");

        REQUIRE_EQ(u.fragment(), "");
    }

}
