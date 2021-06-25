#pragma once

#include "endpoint_http.hpp"
#include "malloy/http/request.hpp"
#include "malloy/http/uri.hpp"

#include <filesystem>

namespace malloy::server
{
    class endpoint_http_files :
        public endpoint_http
    {
        using write_func = writer_t<boost::beast::http::file_body, boost::beast::http::string_body>;
    public:
        std::string resource_base;
        std::filesystem::path base_path;

        write_func writer;
        

        [[nodiscard]]
        bool matches(const req_header_t&, const url_t& url) const override
        {
            return url.resource_starts_with(resource_base);
        }

        [[nodiscard]]
        handle_retr handle(const req_t& req, const http::connection_t& conn) const override
        {
            // Chop request resource path

            std::visit([this, conn](auto& gen) {
                gen->template body<boost::beast::http::string_body>([this, conn](auto&& req) {
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
