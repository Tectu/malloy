#pragma once

#include <filesystem>
#include <string_view>

namespace malloy::http
{
    class response;
    class request;

    /**
     * A generator for HTTP responses.
     *
     * @class generator
     */
    class generator
    {
    public:
        /**
         * Default constructor.
         */
        generator() = default;

        generator(const generator &other) = delete;
        generator(generator &&other) = delete;

        /**
         * Destructor
         */
        virtual ~generator() = default;

        // Operators
        generator &operator=(const generator &rhs) = delete;
        generator &operator=(generator &&rhs) = delete;

        /**
         * Construct a 200 response.
         */
        [[nodiscard]]
        static response ok();

        /**
         * Construct a 400 error.
         *
         * @param reason An explanation of why this request is considered a bad one.
         * @return The response.
         */
        [[nodiscard]]
        static response bad_request(std::string_view reason);

        /**
         * Construct a 404 error.
         *
         * @param resource The resource that was being requested.
         * @return The response.
         */
        [[nodiscard]]
        static response not_found(std::string_view resource);

        /**
         * Construct a 500 error.
         *
         * @param what An optional error message.
         * @return The response.
         */
        [[nodiscard]]
        static response server_error(std::string_view what);

        /**
         * Construct a file response.
         *
         * @param req The request to be responded to.
         * @param storage_path The base path to the local filesystem.
         * @return The response.
         */
        [[nodiscard]]
        static response file(const request& req, const std::filesystem::path& storage_path);

        /**
         * Construct a file response.
         *
         * @param storage_path The base path to the local filesystem.
         * @param rel_path The file being requested relative to the storage_path.
         * @return The response.
         */
        [[nodiscard]]
        static response file(const std::filesystem::path& storage_path, std::string_view rel_path);
    };

}