#pragma once

#include "endpoint_http.hpp"
#include "malloy/http/request.hpp"
#include "malloy/http/uri.hpp"
#include "malloy/server/routing/endpoint_http_regex";

#include <filesystem>

namespace malloy::server
{
    template<typename Req, typename Handler>
    class endpoint_http_files :
        endpoint_http
    {
        using endpt_t = endpoint_http_regex<Req, response_t<boost::beast::http::file_body, boost::beast::http::string_body>, Handler, false>;
        using write_func = writer_t<boost::beast::http::file_body, boost::beast::http::string_body>;
    public:
        std::string resource_base;
        std::filesystem::path base_path;

        void writer(write_func&& f) {
            handoff_.writer = std::move(f);
        }

        [[nodiscard]]
        bool matches(const req_header_t& req) const override
        {
            return malloy::http::uri{std::string{req.target()}}.resource_starts_with(resource_base);
        }

        [[nodiscard]]
        handle_retr handle(const req_t& req, const http::connection_t& conn) const override
        {
            // Chop request resource path

            std::visit([writer, conn](auto& gen) {
                gen->body<boost::beast::http::string_body>([[writer, conn](const auto& req) {
                    malloy::http::request<> req_clone{ req };
                    req_clone.uri().chop_resource(resource_base);

                    // Create response
                    writer(req, malloy::http::generator::file(req_clone, base_path), conn);
                });
            }, req);

            return std::nullopt;
        }
    };

}
