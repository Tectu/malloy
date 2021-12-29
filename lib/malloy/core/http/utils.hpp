#pragma once

#include "types.hpp"
#include "../utils.hpp"

#include <boost/beast/core/string.hpp>
#include <boost/beast/http/message.hpp>

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
        using namespace std::literals;

        return malloy::split(field_value, "; "sv);
    }

}
