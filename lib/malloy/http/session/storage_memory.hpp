#pragma once

#include "storage.hpp"
#include "session.hpp"

#include <unordered_map>

namespace malloy::http::sessions
{

    struct session;

    /**
     * A simple in-memory storage manager.
     */
    struct storage_memory :
        storage
    {

        [[nodiscard]]
        std::shared_ptr<session> create_session(id_type id) override
        {
            auto ses = std::make_shared<session>(
                std::move(id),
                std::bind(
                    &storage_memory::update_session,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3
                )
            );

            m_sessions.emplace(ses->id(), ses);

            return ses;
        }

        [[nodiscard]]
        std::shared_ptr<session> get_session(const id_type& id) override
        {
            if (m_sessions.contains(id))
                return m_sessions.at(id);

            return { };
        }

        void destroy_session(id_type id) override
        {
            m_sessions.erase(id);
        }

        [[nodiscard]]
        bool update_session(
            [[maybe_unused]] const session& ses,
            [[maybe_unused]] const std::string& key,
            [[maybe_unused]] const std::string& value
        ) override
        {
            // Nothing to do here as this is an in-memory session storage and
            // sessions already save the values in their own memory.

            return true;
        }

        std::size_t destroy_expired_sessions(const std::chrono::seconds& max_lifetime) override
        {
            std::size_t count = 0;

            return count;
        }

    private:
        std::unordered_map<id_type, std::shared_ptr<session>> m_sessions;

    };
}
