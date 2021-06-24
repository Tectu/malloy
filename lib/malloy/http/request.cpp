#include "request.hpp"

using namespace malloy::http;

request::request(
    http::method method_,
    std::string_view host,
    const std::uint16_t port,
    std::string_view target_
) :
    m_port(port)
{
    version(11);
    method(method_);
    target(target_);
    set(http::field::host, host);

    // URI
    class uri u{ std::string{ target_.data(), target_.size() }};
    m_uri = std::move(u);
}

request::request(boost::beast::http::request<boost::beast::http::string_body>&& raw)
{
    using namespace boost::beast::http;

    using base_type = boost::beast::http::request<boost::beast::http::string_body>;

    // Underlying
    base_type::operator=(std::move(raw));

    // URI
    class uri u{ std::string{target().data(), target().size()} };
    m_uri = std::move(u);

    // Cookies
    {
        const auto& [begin, end] = base().equal_range(field::cookie);
        for (auto it = begin; it != end; it++) {
            const auto& str = it->value();

            const auto& sep_pos = it->value().find('=');
            if (sep_pos == std::string::npos)
                continue;

            std::string key{ str.substr(0, sep_pos) };
            std::string value{ str.substr(sep_pos+1) };
            m_cookies.insert_or_assign(std::move(key), std::move(value));
        }
    }
}

std::string_view
request::cookie(const std::string_view& name) const
{
    const auto& it = std::find_if(
        std::cbegin(m_cookies),
        std::cend(m_cookies),
        [&name]( const auto& pair ) {
            return pair.first == name;
        }
    );

    if (it == std::cend(m_cookies))
        return { };

    return it->second;
}
