#include "form.hpp"
#include "../utils.hpp"

#include <boost/algorithm/string.hpp>

#include <vector>

using namespace malloy::html;

bool form::parse(const malloy::http::request<>& req, const bool url_decoding)
{
    // Split pairs
    std::vector<std::string> pairs;
    boost::split(pairs, req.body(), boost::is_any_of("&"));

    // Parse pairs
    for (const std::string& pair_str : pairs) {
        std::vector<std::string> pair;
        pair.reserve(2);
        boost::split(pair, pair_str, boost::is_any_of("="));

        if (pair.size() != 2)
            continue;

        // Perform URL decoding (if supposed to)
        std::string value = std::move(pair[1]);
        if (url_decoding)
            url_decode(value);

        m_values.try_emplace(std::move(pair[0]), std::move(value));
    }

    return true;
}

bool form::has_value(const std::string& key) const
{
    return m_values.contains(key);
}

std::optional<std::string> form::value(const std::string& key) const
{
    const auto& it = m_values.find(key);
    if (it == std::cend(m_values))
        return std::nullopt;

    return it->second;
}
