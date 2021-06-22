#pragma once

#include "endpoint_http.hpp"

#include "malloy/http/response.hpp"

#include <functional>
#include <regex>
#include <cassert>

namespace malloy::server
{
    struct resource_matcher {
        [[nodiscard]]
        virtual bool matches_resource(const malloy::http::request& req) const = 0;
    };

    template<typename Response, bool WantsCapture>
    class endpoint_http_regex :
        endpoint_http, public resource_matcher
    {
    public:
        using handler_t = std::conditional_t<
            WantsCapture,
            std::function<Response(const malloy::http::request&,
                                   const std::vector<std::string>&)>,
            std::function<Response(const malloy::http::request&)>>;

        std::regex resource_base;
        handler_t handler;
        std::function<void(const malloy::http::request&, Response&&, const http::connection_t&)> writer;

        [[nodiscard]]
        bool matches_resource(const malloy::http::request& req) const override
        {
            return !match_target(req).empty();
        }

        [[nodiscard]]
        bool matches(const malloy::http::request& req) const override
        {
            // Resource
            if (!matches_resource(req))
                return false;

            // Base class
            return endpoint_http::matches(req);
        }

        [[nodiscard]]
        handle_retr handle(const malloy::http::request& req, const http::connection_t& conn) const override
        {
            if (handler) {
                if constexpr (WantsCapture) {
                    const auto url_matches = match_target(req);
                    if (url_matches.empty()) {
                        throw std::logic_error{
                                R"(endpoint_http_regex passed request which does not match: )" +
                            std::string{req.target()}};
                    }

                    std::vector<std::string> matches{
                    // match_results[0] is the input string
                    url_matches.begin() + 1, url_matches.end()}; 

                    writer(req, handler(req, matches), conn);
                }
                else {
                    writer(req, handler(req), conn);
                }
                return std::nullopt;
            }

            return malloy::http::generator::server_error("no valid handler available.");
        }
    private:
        auto match_target(const malloy::http::request& req) const -> std::smatch {
            std::smatch match_result;
            std::string str{ req.uri().raw() };
            std::regex_match(str, match_result, resource_base);

            return match_result;
        }


    };

}
