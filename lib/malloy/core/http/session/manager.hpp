#pragma once

#include "types.hpp"
#include "../response.hpp"
#include "../request.hpp"

#include <chrono>
#include <memory>
#include <mutex>
#include <string>

namespace malloy::http::sessions
{
    struct storage;
    struct session;

    class manager
    {
    public:
        /**
         * Constructor.
         *
         * This will throw `std::invalid_argument` if no valid storage is provided.
         *
         * @param storage The session storage to use.
         */
        explicit manager(std::shared_ptr<storage> storage);

        /**
         * Copy constructor.
         *
         * @param other
         */
        manager(const manager& other) = delete;

        /**
         * Move constructor.
         *
         * @param other
         */
        manager(manager&& other) noexcept = delete;

        /**
         * Destructor.
         */
        virtual ~manager() = default;

        /**
         * Copy assignment operator.
         *
         * @param rhs
         * @return
         */
        manager& operator=(const manager& rhs) = delete;

        /**
         * Move assignment operator.
         *
         * @param rhs
         * @return
         */
        manager& operator=(manager&& rhs) noexcept = delete;

        /**
         * Get an existing session (if any) or create a new one.
         *
         * @param req The request.
         * @param resp The response.
         * @return The session.
         */
        [[nodiscard]]
        std::shared_ptr<session> start(const request<>& req, response<>& resp);

        /**
         * Destroys an existing session.
         *
         * @param req The request.
         * @param resp The response.
         */
        void destroy(const request<>& req, response<>& resp);

        /**
         * Destroys any sessions older than the specified max lifetime.
         *
         * @param max_lifetime The maximum lifetime of a session.
         * @return The number of sessions that were expired/destroyed.
         */
        std::size_t destroy_expired(const std::chrono::seconds& max_lifetime);

    private:
        std::shared_ptr<storage> m_storage;
        std::mutex m_lock; // protects sessions
        std::string m_cookie_name = "sessionId";      // The name of the session cookie

        /**
         * Generates a new, unique session ID
         */
        [[nodiscard]]
        static id_type generate_id();
    };

}
