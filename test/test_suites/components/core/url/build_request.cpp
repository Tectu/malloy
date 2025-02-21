#include "../../../../test.hpp"

#include <malloy/core/http/url.hpp>

TEST_SUITE("components - core - url - build_request()")
{

    TEST_CASE("url string")
    {
        SUBCASE("#1")
        {
            auto req = malloy::http::build_request<boost::beast::http::string_body>(malloy::http::method::get, "http://example.com/");
            REQUIRE(req);

            CHECK_EQ(req->method(), malloy::http::method::get);
            CHECK_EQ(req->use_tls(), false);
            CHECK_EQ(req->port(), 80);
            CHECK_EQ(req->base()[malloy::http::field::host], "example.com");
            CHECK_EQ(req->target(), "/");
        }

        SUBCASE("#2")
        {
            auto req = malloy::http::build_request<boost::beast::http::empty_body>(malloy::http::method::get, "https://example.com/foo/bar?one=1&two=2");
            REQUIRE(req);

            CHECK_EQ(req->method(), malloy::http::method::get);
            CHECK_EQ(req->use_tls(), true);
            CHECK_EQ(req->port(), 443);
            CHECK_EQ(req->base()[malloy::http::field::host], "example.com");
            CHECK_EQ(req->target(), "/foo/bar?one=1&two=2");
        }

        SUBCASE("#3")
        {
            auto req = malloy::http::build_request<boost::beast::http::empty_body>(malloy::http::method::get, "https://example.com:4242/foo#frag");
            REQUIRE(req);

            CHECK_EQ(req->method(), malloy::http::method::get);
            CHECK_EQ(req->use_tls(), true);
            CHECK_EQ(req->port(), 4242);
            CHECK_EQ(req->base()[malloy::http::field::host], "example.com");
            CHECK_EQ(req->target(), "/foo");
        }
    }
}
