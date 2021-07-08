#include "../../test.hpp"
#include "../../mocks.hpp" 

#include <malloy/core/http/request.hpp>
#include <malloy/server/routing/endpoint_http_regex.hpp>
#include <malloy/server/routing/router.hpp>

using namespace malloy::http;
using namespace malloy::server;


void endpt_handle(const auto& endpt, const std::string& url) {
    malloy::http::request_header<> reqh;
    reqh.target(url);
    reqh.method(method::get);
    [[maybe_unused]] const auto rs = endpt.handle(std::make_shared<malloy::mock::http::connection::request_generator>(reqh), http::connection_t{ std::shared_ptr<http::connection_plain>{nullptr} });
}

TEST_SUITE("components - endpoints") {
    TEST_CASE("An http_regex_endpoint with a handler that takes "
              "std::vector<std::string> as an additional param will pass the "
              "values of the capture groups in the input regex as elements of "
              "that vector")
    {
        constexpr auto first_cap = "thisisaword";
        constexpr auto second_cap = "42";

        const auto input_url = std::string{"/content/"} + first_cap + "/" + second_cap;
        std::regex input_reg{R"(/content/(\w+)/(\d+))"};
        endpoint_http_regex<response<>, malloy::server::detail::default_route_filter, true> endpt;

        bool handler_called{false};
        endpt.handler = [&, called = &handler_called](const auto& req, const std::vector<std::string>& results) {
            CHECK(results.size() == 2);
            CHECK(results.at(0) == first_cap);
            CHECK(results.at(1) == second_cap);
            (*called) = true;

            return generator::ok();
        };
        endpt.resource_base = input_reg;
        endpt.writer = [](auto&&...){};

        endpt_handle(endpt, input_url);
        CHECK(handler_called);
        
    }
    TEST_CASE("An endpoint_http_regex with a handler that only accepts malloy::http::request will not try to pass capture group values"){
        constexpr auto input_url = "/content/word";
        const auto input_reg{R"(/content/(\w+))"};

        endpoint_http_regex<response<>, malloy::server::detail::default_route_filter, false> endpt;
        bool handler_called{false};
        endpt.handler = [called = &handler_called](const auto& req) {
            (*called) = true;
            return generator::ok();
        };
        endpt.resource_base = input_reg;
        endpt.writer = [](auto&&...) {};

        endpt_handle(endpt, input_url);
        CHECK(handler_called);
        


    }
}

