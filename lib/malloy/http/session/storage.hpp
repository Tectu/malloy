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
            [[maybe_unused]] const session::key_type& key,
            [[maybe_unused]] const session::value_type& value
        ) = 0;
    };

}
