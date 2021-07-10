#pragma once

#include <boost/beast/core/string_view.hpp>

namespace malloy::http
{
    inline auto resource_string(boost::beast::string_view target) -> boost::beast::string_view
    {
        // Taken from: https://github.com/Tectu/malloy/blob/f6b06b25ba7f78f81e22d077f64acb95c0551d88/lib/malloy/core/http/uri.cpp#L72
        const auto pos = target.find_first_of("?#");
        if (pos == std::string::npos) {
            return target;
        } else {
            return target.substr(0, pos);
        }
    }

}


