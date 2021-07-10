#pragma once

#include "endpoint_http.hpp"
#include "../../core/http/request.hpp"
#include "malloy/core/http/utils.hpp"

#include <filesystem>

namespace malloy::server
{
    /**
     * @brief Endpoint for file serving
     * @details Serves files at a resource path. The url path after the resource
     * base is appended to a base path on the filesystem. e.g. /content/img.svg
     * with a resource path of / and a base path of /var/www/content would
     * result in the file at /var/www/content/content/img.svg being served.
     */
    class endpoint_http_files :
        public endpoint_http
    {
        using write_func = writer_t<boost::beast::http::file_body, boost::beast::http::string_body>;

    public:
        std::string resource_base;
        std::filesystem::path base_path;

        write_func writer;

        [[nodiscard]]
        bool matches(const req_header_t& head) const override
        {
            return malloy::http::resource_string(head).starts_with(resource_base);
        }

        [[nodiscard]]
        handle_retr handle(const req_t& req, const http::connection_t& conn) const override
        {
            std::visit([this, conn](auto& gen) {
                gen->template body<boost::beast::http::string_body>([this, conn](auto&& req) {
                    malloy::http::request<> req_clone{ req };
                    malloy::http::chop_resource(req_clone, resource_base);

                    // Create response
                    writer(req, malloy::http::generator::file(req_clone, base_path), conn);
                    });
                },
                req
            );

            return std::nullopt;
        }
    };

}
