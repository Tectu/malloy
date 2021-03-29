#include "router.hpp"

#include <stdexcept>

using namespace malloy::http::server;

router::router(std::shared_ptr<spdlog::logger> logger) :
        m_logger(std::move(logger))
{
    // Sanity check
    if (not m_logger)
        throw std::runtime_error("received invalid logger instance.");
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string
router::path_cat(beast::string_view base, beast::string_view path)
{
    if(base.empty())
        return std::string(path);
    std::string result(base);
#ifdef BOOST_MSVC
    char constexpr path_separator = '\\';
        if(result.back() == path_separator)
            result.resize(result.size() - 1);
        result.append(path.data(), path.size());
        for(auto& c : result)
            if(c == '/')
                c = path_separator;
#else
    char constexpr path_separator = '/';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif
    return result;
}
