#pragma once

#include <filesystem>
#include <string_view>

namespace malloy::http
{
    class response;
    class request;

    class generator
    {
    public:
        // Construction
        generator() = default;
        generator(const generator &other) = delete;
        generator(generator &&other) = delete;
        virtual ~generator() = default;

        // Operators
        generator &operator=(const generator &rhs) = delete;
        generator &operator=(generator &&rhs) = delete;

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