#pragma once

#include "types.hpp"

#include <string>
#include <unordered_map>

namespace malloy::http::sessions
{

    struct session
    {
        using key_type   = std::string;
        using value_type = std::string;
        using id_type    = std::string;

        session(id_type&& id) :
            m_id(std::move(id))
        {
        }

        void set(const key_type& key, value_type value)
        {
            m_data.insert_or_assign(key, value);
        }

        std::optional<key_type> get(const key_type& key) const
        {
            const auto& it = m_data.find(key);
            if (it != std::cend(m_data))
                return it->second;
            return std::nullopt;
        }

        bool remove(const key_type& key)
        {
            return m_data.erase(key) > 0;
        }

        [[nodiscard]]
        id_type id() const noexcept
        {
            return m_id;
        }

    private:
        id_type m_id;
        std::unordered_map<key_type, value_type> m_data;
    };

}
