#pragma once

#include "types.hpp"

#include <functional>
#include <string>
#include <unordered_map>

namespace malloy::http::sessions
{

    /**
     * This class represents a session.
     *
     * This is a classic implementation for achieving a stateful session over stateless HTTP.
     * It works by generating a server-side session using an object of this class and sending the
     * session ID to the client as a cookie. The client will send back this cookie on sub-sequent
     * requests through which the server can retrieve the corresponding session ID.
     *
     * Note that the session cookie should be marked 'Secure' and 'HttpOnly' to increase protection
     * on the client side.
     */
    struct session
    {
        using key_type    = std::string;
        using value_type  = std::string;
        using id_type     = std::string;
        using update_cb_t = std::function<void(const session&, const key_type& key, const value_type& value)>;

        session(id_type&& id, update_cb_t update_cb) :
            m_id(std::move(id)),
            m_update_cb(std::move(update_cb))
        {
            if (not m_update_cb)
                throw std::logic_error("no valid update call-back provided.");
        }

        void set(const key_type& key, value_type value)
        {
            m_data.insert_or_assign(key, value);

            if (m_update_cb)
                m_update_cb(*this, key, value);
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
        update_cb_t m_update_cb;
        std::unordered_map<key_type, value_type> m_data;
    };

}
