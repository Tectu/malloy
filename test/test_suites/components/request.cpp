#include "../../test.hpp"

#include <malloy/http/request.hpp> 

using namespace malloy::http;
namespace bhttp = boost::beast::http;

TEST_SUITE("components - request") {
    TEST_CASE("ctor") {
        const std::string target = "/test";

        SUBCASE("Constructing a request with a beast request results in a uri matching the target") {
            bhttp::request<bhttp::string_body> req;
            req.target(target);

            request mreq{std::move(req)};

            CHECK(mreq.uri().raw() == target);
        }

        SUBCASE("Constructing a request with a target string results in a uri matching the target") {
            request req{method::get, "", 0, target};
            CHECK(req.uri().raw() == target);
        }
    }

}

