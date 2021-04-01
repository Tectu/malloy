#include "uri.hpp"

#include <boost/algorithm/string.hpp>

#include <sstream>

using namespace malloy::http;

bool uri::chop_resource(const std::string_view str)
{
    // Sanity check
    if (not m_raw.starts_with(str) or not m_resource_string.starts_with(str))
        return false;

    // Modify the resource string
    m_raw = m_raw.substr(str.size());
    m_resource_string = m_resource_string.substr(str.size());

    // Re-parse the resource string
    parse_resource();

    return true;
}

std::string uri::to_string() const
{
    std::stringstream str;

    str << "raw: " << m_raw << "\n";
    str << "resource:\n";
    str << "  string: " << m_resource_string << "\n";
    for (std::size_t i = 0; i < m_resource.size(); i++)
        str << "  resource[" << i << "]: " << m_resource.at(i) << "\n";
    str << "query:\n";
    str << "  string: " << m_query_string << "\n";
    for (const auto& [qry_key, qry_value] : m_query)
        str << "  " << qry_key << " = " << qry_value << "\n";
    str << "fragment: " << m_fragment << "\n";

    return str.str();
}

void uri::parse()
{
    // Split target
    {
        // Create a string view on the raw string
        const auto raw_sv = std::string_view{ m_raw };

        const auto pos = raw_sv.find_first_of("?#");
        if (pos == std::string_view::npos) {
            m_resource_string = raw_sv;
            m_query_string = { };
            m_fragment = { };
        }
        else {
            m_resource_string = raw_sv.substr(0, pos);
            m_query_string = raw_sv.substr(pos+1);
            m_fragment = { };       // ToDo!
        }
    }

    // Resource
    parse_resource();

    // Query
    parse_query();
}

void uri::parse_resource()
{
    // Clear
    m_resource.clear();

    // Split
    if (not m_resource_string.empty()) {
        std::string_view str = m_resource_string.substr(1);
        boost::split(m_resource, str, boost::is_any_of("/"));

        // Ignore if it's "/" or ""
        if (m_resource_string.size() == 1 and m_resource_string.at(0) == '/')
            m_resource.clear();
    }
}

void uri::parse_query()
{
    // Clear
    m_query.clear();

    // Split
    std::vector<std::string_view> strings_split;
    boost::split(strings_split, m_query_string, boost::is_any_of("&#"));

    // Parse
    for (const std::string_view& str : strings_split) {
        std::vector<std::string_view> key_value;
        boost::split(key_value, str, boost::is_any_of("="));

        if (key_value.size() != 2)
            continue;

        m_query.insert_or_assign(key_value[0], key_value[1]);
    }
}
