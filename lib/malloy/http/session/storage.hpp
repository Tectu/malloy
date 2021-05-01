#pragma once

#include "types.hpp"

#include <memory>

namespace malloy::http::sessions
{

    struct session;

    /**
     * A session storage interface.
     */
    struct storage
    {
        /**
         * Create a new session.
         *
         * @param id The session ID.
         * @return The created session.
         */
        [[nodiscard]]
        virtual std::shared_ptr<session> create_session(id_type id) = 0;

        /**
         * Returns an existing session (if any)
         *
         * @param id The session ID.
         * @return The session (if any).
         */
        [[nodiscard]]
        virtual std::shared_ptr<session> get_session(const id_type& id) = 0;

        /**
         * Destroy an existing session.
         *
         * @param id The session ID.
         */
        virtual void destroy_session(id_type id) = 0;

        /**
         * Session values were modified. Update the underlying storage accordingly.
         *
         * @param ses The session.
         * @param key The key.
         * @param value The new value.
         * @return Whether the update was successful.
         */
        [[nodiscard]]
        virtual bool update_session(
            [[maybe_unused]] const session& ses,
            [[maybe_unused]] const std::string& key,
            [[maybe_unused]] const std::string& value
        ) = 0;

        /**
         * Destroy any session older than a specified value.
         * Sessions older than `max_lifetime` need to be destroyed.
         *
         * @note The session manager guarantees that max_lifetime is greater than zero.
         *
         * @param max_lifetime The maximum lifetime of a session.
         * @return The number of sessions that were expired/destroyed.
         */
        virtual std::size_t destroy_expired_sessions(const std::chrono::seconds& max_lifetime) = 0;
    };

}
