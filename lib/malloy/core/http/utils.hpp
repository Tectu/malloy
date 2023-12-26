#pragma once

#include "types.hpp"
#include "../utils.hpp"

#include <boost/beast/core/string.hpp>
#include <boost/beast/http/message.hpp>

#include <optional>

namespace malloy::http
{

    template<bool isReq, typename Fields>
    [[nodiscard]]
    std::string_view
    resource_string(const boost::beast::http::header<isReq, Fields>& header)
    {
        const auto target = header.target();

        const auto pos = target.find_first_of("?#");
        if (pos == std::string::npos)
            return target;
        else
            return target.substr(0, pos);
    }

    template<bool isReq, typename Fields>
    void
    chop_resource(boost::beast::http::header<isReq, Fields>& head, std::string_view resource)
    {
        head.target(head.target().substr(resource.size()));
    }

    template<bool isReq, typename Fields>
    [[nodiscard]]
    bool
    has_field(const boost::beast::http::header<isReq, Fields>& head, const malloy::http::field check)
    {
        return head.find(check) != head.end();
    }

    /**
     * Split a header value into its individual parts.
     *
     * Example:
     *   input:  "multipart/form-data; boundary=----WebKitFormBoundarynBjZTMv9eqwyCWhj"
     *   output:
     *     - "multipart/form-data"
     *     - "boundary=----WebKitFormBoundarynBjZTMv9eqwyCWhj"
     *
     * @param field_value The value to split.
     * @return The split values
     *
     * @sa malloy::split(std::string_view, std::string_view)
     */
    [[nodiscard]]
    inline
    std::vector<std::string_view>
    split_header_value(std::string_view field_value)
    {
        using namespace std::string_view_literals;

        return malloy::split(field_value, "; "sv);
    }

    /**
     * Extracts a cookie value (if any).
     *
     * @tparam isReq
     * @tparam Fields
     * @param header The HTTP header.
     * @param cookie_name The cookie name.
     * @return The cookie value (if any).
     */
    // ToDo: This implementation could use some love. It's comparably inefficient as we're splitting first on each
    //       cookie value separation ("; ") and then on each value-pair separator ("="). A more elegant implementation
    //       would just continuously advance through the string.
    template<bool isReq, typename Fields>
    [[nodiscard]]
    std::optional<std::string_view>
    cookie_value(const boost::beast::http::header<isReq, Fields>& header, const std::string_view cookie_name)
    {
        using namespace std::string_view_literals;

        // Get cookie field
        const auto& it = header.find(field::cookie);
        if (it == header.cend())
            return std::nullopt;

        // Split pairs
        const auto& pairs = split_header_value(it->value());

        // Check each pair
        for (const std::string_view& pair : pairs) {
            // Split
            const auto& parts = malloy::split(pair, "="sv);
            if (parts.size() != 2)
                continue;

            // Check cookie name
            if (parts[0] == cookie_name)
                return parts[1];
        }

        return std::nullopt;
    }

}
