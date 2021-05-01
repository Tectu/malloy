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
        virtual std::shared_ptr<session> create(id_type id) = 0;

        /**
         * Returns an existing session (if any)
         *
         * @param id The session ID.
         * @return The session (if any).
         */
        [[nodiscard]]
        virtual std::shared_ptr<session> get(const id_type& id) = 0;

        /**
         * Destroy an existing session.
         *
         * @param id The session ID.
         */
        virtual void destroy(id_type id) = 0;

        /**
         * Destroy any session older than a specified value.
         * Sessions older than `max_lifetime` need to be destroyed.
         *
         * @note The session manager guarantees that max_lifetime is greater than zero.
         *
         * @param max_lifetime The maximum lifetime of a session.
         * @return The number of sessions that were expired/destroyed.
         */
        virtual std::size_t destroy_expired(const std::chrono::seconds& max_lifetime) = 0;
    };

}
