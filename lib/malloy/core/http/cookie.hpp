#pragma once

#include <chrono>
#include <filesystem>
#include <string>

namespace malloy::http
{

    /**
     * The cookie class.
     *
     * This class represents a cookie.
     *
     * @note Implementation based on: https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Set-Cookie
     */
    class cookie
    {
    public:
        enum class same_site_t
        {
            strict,
            lax,
            none
        };

        std::string name;
        std::string value;
        std::chrono::seconds max_age;
        std::string domain;
        std::filesystem::path path;
        bool secure                     = true;
        bool http_only                  = true;
        same_site_t same_site           = same_site_t::strict;

        [[nodiscard]]
        std::string to_string() const;
    };

}