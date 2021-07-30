#pragma once

#include "../http/request.hpp"

#include <optional>
#include <string>
#include <vector>

namespace malloy::html
{

    /**
     * A parser for parsing multipart/form-data HTTP bodies.
     *
     * This should be able to parse according to RFC7578.
     */
    class multipart_parser
    {
    public:
        /**
         * Type to represent a single part of a multipart/form-data message.
         */
        struct part
        {
            std::string_view disposition;
            std::string_view type;
            std::string_view content;

            part() = default;
            part(const part&) = default;
            part(part&&) noexcept = default;
            virtual ~part() = default;

            part& operator=(const part&) = default;
            part& operator=(part&&) noexcept = default;
        };

        /**
         * Parses the individual parts of a multipart/form-data message.
         *
         * @param req The request to parse.
         * @return The individual, parsed parts.
         */
        [[nodiscard]]
        static
        std::vector<part>
        parse(const malloy::http::request<>& req);

        /**
         * Parses the individual parts of a multipart/form-data message.
         *
         * @param body The request body.
         * @param boundary The boundary indicator.
         * @return The individual, parsed parts.
         */
        [[nodiscard]]
        static
        std::vector<part>
        parse(std::string_view body, const std::string& boundary);

    private:
        constexpr static std::string_view str_disposition = { "Content-Disposition: " };
        constexpr static std::string_view str_type        = { "Content-Type: " };

        [[nodiscard]]
        static
        std::optional<part>
        parse_part(std::string_view part_raw);
    };

}
