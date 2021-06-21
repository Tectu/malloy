#pragma once

#include "endpoint_http.hpp"

#include "malloy/http/response.hpp"
#include "malloy/server/routing/type_traits.hpp"

#include <functional>
#include <regex>

namespace malloy::server
{
    struct resource_matcher {
        [[nodiscard]]
        virtual bool matches_resource(const boost::beast::http::request_header<>& req) const = 0;
    };

    template<typename Req, typename Response, typename Handler>
    struct endpoint_http_regex :
        endpoint_http, public resource_matcher
    {
        using handler_t = Handler;

        std::regex resource_base;
        handler_t handler;
        std::function<void(const Req&, Response&&, const http::connection_t&)> writer;

        [[nodiscard]]
        bool matches_resource(const req_header_t& req) const override
        {
            std::smatch match_result;
            std::string str{ req.target() };
            return std::regex_match(str, match_result, resource_base);
        }

        [[nodiscard]]
        bool matches(const req_header_t& req) const override
        {
            // Resource
            if (!matches_resource(req))
                return false;

            // Base class
            return endpoint_http::matches(req);
        }

        [[nodiscard]]
        handle_retr handle(const req_t& gens, const http::connection_t& conn) const override
        {
          if (handler) {
              std::visit(
                  [this, conn](auto& gen) {
                      auto bodies = [this, gen] {
                          if constexpr (concepts::advanced_route_handler<
                                            Handler, Req>) {
                              return handler.body_for(gen.header());
                          }
                          else {
                              return std::variant<
                                  boost::beast::http::string_body>{
                                  boost::beast::http::string_body{}};
                          }
                      }();
                      std::visit([this, gen, conn](auto&& body) mutable {
                          using T = std::decay_t<decltype(body)>;
                          gen.template body<T>(
                              std::forward<decltype(body)>(body),
                              [this, conn](const auto& req) {
                                  writer(req, handler(req), conn);
                              });
                      }, std::move(bodies));
                  },
                  gens);
              return std::nullopt;
          }

            return malloy::http::generator::server_error("no valid handler available.");
        }

    };

}
