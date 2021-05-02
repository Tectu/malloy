#pragma once

#include "storage.hpp"
#include "session.hpp"

#include <chrono>
#include <unordered_map>

namespace malloy::http::sessions
{

    struct session;

    /**
     * A simple in-memory storage manager.
     */
    class storage_memory :
        public storage
    {
    private:
        /**
         * A session with implemented storage interface.
         */
        struct record :
            session_chrono<std::chrono::steady_clock>
        {

            explicit record(id_type&& id) :
                session_chrono(std::move(id))
            {
            }

            bool storage_set(const key_type& key, value_type value) override
            {
                try {
                    m_data.insert_or_assign(key, value);
                }
                catch (...)
                {
                    return false;
                }

                return true;
            }

            [[nodiscard]]
            std::optional<key_type> storage_get(const key_type& key) const override
            {
                const auto& it = m_data.find(key);
                if (it != std::cend(m_data))
                    return it->second;
                return std::nullopt;
            }

            [[nodiscard]]
            bool storage_remove(const key_type& key) override
            {
                return m_data.erase(key) > 0;
            }

        private:
            std::unordered_map<std::string, std::string> m_data;
        };

    public:
        [[nodiscard]]
        std::shared_ptr<session> create(id_type id) override
        {
            auto ses = std::make_shared<record>(
                std::move(id)
            );

            m_sessions.try_emplace(ses->id(), ses);

            return ses;
        }

        [[nodiscard]]
        std::shared_ptr<session> get(const id_type& id) override
        {
            if (not m_sessions.contains(id))
                return { };

            return m_sessions.at(id);
        }

        void destroy(id_type id) override
        {
            m_sessions.erase(id);
        }

        std::size_t destroy_expired(const std::chrono::seconds& max_lifetime) override
        {
            // ToDo: This can be upgraded to use std::erase_if() once C++20 support is good enough.

            std::size_t count = 0;
            for (auto it = std::begin(m_sessions); it not_eq std::end(m_sessions);) {
                if (it->second->access_time_older_than(max_lifetime)) {
                    it = m_sessions.erase(it);
                    ++count;
                }
                else
                    ++it;
            }

            return count;
        }

    private:
        std::unordered_map<id_type, std::shared_ptr<record>> m_sessions;
    };
}
