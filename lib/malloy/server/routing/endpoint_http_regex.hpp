#pragma once

#include "endpoint_http.hpp"

#include "malloy/http/response.hpp"
#include "malloy/type_traits.hpp"
#include "malloy/server/routing/type_traits.hpp"

#include <functional>
#include <regex>
#include <cassert>

namespace malloy::server
{
    struct resource_matcher {
        [[nodiscard]]
        virtual bool matches_resource(const boost::beast::http::request_header<>& req) const = 0;
    };

    template<typename Response, concepts::route_filter Handler, bool WantsCapture>
    class endpoint_http_regex :
        public endpoint_http, public resource_matcher
    {
    public:
        template<typename Derived>
        using req_gen_t = std::shared_ptr<typename http::connection<Derived>::request_generator>;
        template<typename Req>
        using handler_t = std::conditional_t<
            WantsCapture,
            std::function<Response(const Req&,
                const std::vector<std::string>&)>,
            std::function<Response(const Req&)>>;


        Handler filter;
        std::regex resource_base;
        handler_t<typename Handler::request_type> handler;
        std::function<void(const boost::beast::http::request_header<>&, Response&&, const http::connection_t&)> writer;

        [[nodiscard]]
        bool matches_resource(const req_header_t& req) const override
        {

            return std::regex_match(req.target().begin(), req.target().end(), resource_base);
        }

        [[nodiscard]]
        bool matches(const req_header_t& req, const url_t& url) const override
        {
            // Resource
            if (!matches_resource(req))
                return false;

            // Base class
            return endpoint_http::matches(req, url);
        }


        [[nodiscard]] handle_retr
            handle(const req_t& gens, const http::connection_t& conn) const override
        {
            if (handler) {
                std::visit([this, conn]<typename Generator>(Generator && gen) {
                    visit_bodies(std::forward<Generator>(gen), conn);
                },
                    gens);
                return std::nullopt;
            }
            else {
                return malloy::http::generator::server_error("no valid handler available.");
            }
        }
    private:
        void handle_req(const auto& req, const http::connection_t& conn) const
        {
            if constexpr (WantsCapture) {
                std::smatch url_matches;
                std::string url{ req.target() };
                std::regex_match(url, url_matches, resource_base);

                if (url_matches.empty()) {
                    throw std::logic_error{
                        R"(endpoint_http_regex passed request which does not match: )" +
                        std::string{req.target()} };
                }

                std::vector<std::string> matches{// match_results[0] is the
                                                 // input
                                                 // string
                                                 url_matches.begin() + 1,
                                                 url_matches.end() };

                writer(req, handler(req, matches), conn);
            }
            else {
                writer(req, handler(req), conn);
            }
        }

        void visit_bodies(const auto& gen, const http::connection_t& conn) const
        {
            using body_t = typename Handler::request_type::body_type;
            gen->template body<body_t>(
                [this, conn](const auto& req) {
                    handle_req(req, conn);
                }, [&gen, this](auto& body) { filter.setup_body(gen->header(), body); });
        }
    };

}
