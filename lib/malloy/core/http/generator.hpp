#pragma once

#include "response.hpp"
#include "request.hpp"
#include "type_traits.hpp"
#include "types.hpp"
#include "utils.hpp"

#include <filesystem>
#include <string_view>
#include <variant>

namespace malloy::http
{

    /**
     * A generator for HTTP responses.
     *
     * @class generator
     */
    class generator
    {
        /**
         * A file response can either have an underlying file body or an underlying string body.
         *
         * - A file body allows sending a file from the file system as a response without loading it completely into memory.
         * - A string body can be used to serve a file with contents from memory.
         */
        using file_response = std::variant<response<boost::beast::http::file_body>, response<boost::beast::http::string_body>>;

    public:
        /**
         * Default constructor.
         */
        generator() = default;

        generator(const generator& other) = delete;
        generator(generator&& other) = delete;

        /**
         * Destructor
         */
        virtual ~generator() = default;

        // Operators
        generator& operator=(const generator& rhs) = delete;
        generator& operator=(generator&& rhs) = delete;

        /**
         * Construct a 200 response.
         */
        [[nodiscard]]
        static response<> ok();

        /**
         * Construct a 3xx response.
         *
         * @param code The HTTP status code. Must be a 3xx status code.
         * @param location The location to redirect to.
         * @return The response.
         */
        [[nodiscard]]
        static response<> redirect(status code, std::string_view location);

        /**
         * Construct a 400 error.
         *
         * @param reason An explanation of why this request is considered a bad one.
         * @return The response.
         */
        [[nodiscard]]
        static response<> bad_request(std::string_view reason);

        /**
         * Construct a 404 error.
         *
         * @param resource The resource that was being requested.
         * @return The response.
         */
        [[nodiscard]]
        static response<> not_found(std::string_view resource);

        /**
         * Construct a 500 error.
         *
         * @param what An optional error message.
         * @return The response.
         */
        [[nodiscard]]
        static response<> server_error(std::string_view what);

        /**
         * Construct a file response.
         *
         * @param req The request to be responded to.
         * @param storage_base_path The base path to the local filesystem.
         * @return The response.
         */
        template<malloy::http::concepts::body Body>
        [[nodiscard]]
        static auto file(const request<Body>& req, const std::filesystem::path& storage_base_path) -> file_response {
	        return file(storage_base_path, malloy::http::resource_string(req));
        }

        /**
         * Construct a file response.
         *
         * @param storage_path The base path to the local filesystem.
         * @param rel_path The file being requested relative to the storage_path.
         * @return The response.
         */
        [[nodiscard]]
        static auto file(const std::filesystem::path& storage_path, std::string_view rel_path) -> file_response;
    };

}
