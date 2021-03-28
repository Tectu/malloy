#include "uri.hpp"

#include <boost/algorithm/string.hpp>

using namespace malloy::http;

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

    // Query
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
}