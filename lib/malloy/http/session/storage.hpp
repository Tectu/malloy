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
         * @param id
         * @return
         */
        [[nodiscard]]
        virtual std::shared_ptr<session> create_session(id_type id) = 0;

        /**
         * Returns an existing session (if any)
         *
         * @param id
         * @return
         */
        [[nodiscard]]
        virtual std::shared_ptr<session> get_session(const id_type& id) = 0;

        /**
         * Destroy an existing session.
         *
         * @param id
         */
        virtual void destroy_session(id_type id) = 0;
    };

}
