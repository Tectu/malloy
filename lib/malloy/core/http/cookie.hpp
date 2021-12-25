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
     * @note If both `Max-Age` and `Expires` are set the client is supposed to give `Max-Age` priority/precedence.
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
        std::chrono::seconds max_age{ 0 };
        std::string expires;
        std::string domain;
        std::filesystem::path path;
        bool secure                     = true;
        bool http_only                  = true;
        same_site_t same_site           = same_site_t::strict;

        [[nodiscard]]
        std::string to_string() const;
    };

    /**
     * A cookie which will motivate the client to discard a previously stored cookie with the same name.
     *
     * According to RFC6265 a client must delete a cookie if receiving a Set-Cookie header with an 'Expires` value in the past.
     */
    class cookie_clear :
        public cookie
    {
    public:
        explicit
        cookie_clear(const std::string& name_)
        {
            name = name_;
            expires = m_expired_date;
        }

    private:
        static constexpr const char* m_expired_date = "Thu, 01 Jan 1970 00:00:00 GMT";
    };

}