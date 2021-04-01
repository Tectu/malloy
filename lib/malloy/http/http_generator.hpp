#pragma once

#include <filesystem>
#include <string_view>

namespace malloy::http
{
    class response;
    class request;

    class http_generator
    {
    public:
        // Construction
        http_generator() = default;
        http_generator(const http_generator &other) = delete;
        http_generator(http_generator &&other) = delete;
        virtual ~http_generator() = default;

        // Operators
        http_generator &operator=(const http_generator &rhs) = delete;
        http_generator &operator=(http_generator &&rhs) = delete;

        [[nodiscard]]
        static response bad_request(std::string_view reason);

        [[nodiscard]]
        static response not_found(std::string_view resource);

        [[nodiscard]]
        static response server_error(std::string_view what);

        [[nodiscard]]
        static response file(const request& req, const std::filesystem::path& storage_path);

        [[nodiscard]]
        static response file(const std::filesystem::path& storage_path, std::string_view rel_path);
    };

}