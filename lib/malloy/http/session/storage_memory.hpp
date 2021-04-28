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
            auto ses = std::make_shared<session>(std::move(id));

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

    private:
        std::unordered_map<id_type, std::shared_ptr<session>> m_sessions;

    };

}
