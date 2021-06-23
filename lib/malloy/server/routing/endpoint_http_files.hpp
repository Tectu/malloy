#pragma once

#include "endpoint_http.hpp"
#include "malloy/http/request.hpp"
#include "malloy/http/uri.hpp"

#include <filesystem>

namespace malloy::server
{

    struct endpoint_http_files :
        endpoint_http
    {
    public:
        std::string resource_base;
        std::filesystem::path base_path;
        writer_t<boost::beast::http::file_body, boost::beast::http::string_body> writer;

        [[nodiscard]]
        bool matches(const req_header_t& req) const override
        {
            return false;
            //return malloy::http::uri{std::string{req.target()}}.resource_starts_with(resource_base);
        }

        [[nodiscard]]
        handle_retr handle(const req_t& req, const http::connection_t& conn) const override
        {
     /*       // Chop request resource path
            malloy::http::request<> req_clone{ req };
            req_clone.uri().chop_resource(resource_base);

            // Create response
            writer(req, malloy::http::generator::file(req_clone, base_path), conn);
*/
            return std::nullopt;
        }

    };

}
