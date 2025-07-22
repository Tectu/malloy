#pragma once

#include "endpoint_http.hpp"
#include "../../core/http/request.hpp"
#include "../../core/http/utils.hpp"
#include "../../core/type_traits.hpp"

#include <filesystem>

namespace malloy::server
{
    /**
     * @brief Endpoint for file serving
     *
     * @details Serves files at a resource path. The url path after the resource
     *          base is appended to a base path on the filesystem. e.g. /content/img.svg
     *          with a resource path of / and a base path of /var/www/content would
     *          result in the file at /var/www/content/content/img.svg being served.
     */
    class endpoint_http_files :
        public endpoint_http
    {
        using write_func = writer_t<boost::beast::http::file_body, boost::beast::http::string_body>;

    public:
        std::string resource_base;
        std::filesystem::path base_path;
        std::string cache_control;

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
                    auto resp = malloy::http::generator::file(req_clone, base_path);
                    std::visit([this]<typename Resp>(Resp& resp){ resp.set(malloy::http::field::cache_control, cache_control); }, resp);  // Add Cache-Control header

                    // Send
                    writer(req, std::move(resp), conn);
                    });
                },
                req
            );

            return std::nullopt;
        }
    };

}
