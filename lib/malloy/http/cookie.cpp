#include "cookie.hpp"

#include <sstream>

using namespace malloy::http;

std::string cookie::to_string() const
{
    std::ostringstream ss;

    ss << name << "=" << value << ";";

    if (max_age not_eq std::chrono::seconds::zero())
        ss << "Max-Age=" << max_age.count() << ";";

    if (!domain.empty())
        ss << "Domain=" << domain << ";";

    if (!path.empty())
        ss << "Path=" << path << ";";

    ss << "SameSite=";
    switch (same_site) {
        case same_site_t::strict:   ss << "Strict"; break;
        case same_site_t::lax:      ss << "Lax"; break;
        case same_site_t::none:     ss << "None"; break;
    }
    ss << ";";

    if (secure || same_site == same_site_t::none)
        ss << "Secure;";

    if (http_only)
        ss << "HttpOnly;";

    // ToDo: Ensure that we're not sending a trailing ;

    return ss.str();
}
