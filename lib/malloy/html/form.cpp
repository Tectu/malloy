#include "form.hpp"
#include "../http/request.hpp"

#include <boost/algorithm/string.hpp>

#include <vector>

using namespace malloy::html;

bool form::parse(const malloy::http::request<>& req)
{
    // Split pairs
    std::vector<std::string> pairs;
    boost::split(pairs, req.body(), boost::is_any_of("&"));

    // Parse pairs
    for (const std::string& pair_str : pairs) {
        std::vector<std::string> pair;
        pair.reserve(2);
        boost::split(pair, pair_str, boost::is_any_of("="));

        if (pair.size() not_eq 2)
            continue;

        m_values.try_emplace(std::move(pair[0]), std::move(pair[1]));
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
