#pragma once

#include "types.hpp"

#include <chrono>
#include <memory>
#include <mutex>
#include <string>

namespace malloy::http
{
    class request;
    class response;
}

namespace malloy::http::sessions
{
    struct storage;
    struct session;

    class manager
    {
    public:
        manager() = default;

        manager(const manager& other) = delete;
        manager(manager&& other) noexcept = delete;
        virtual ~manager() = default;

        manager& operator=(const manager& rhs) = delete;
        manager& operator=(manager&& rhs) noexcept = delete;

        bool init(
            std::shared_ptr<storage> storage,
            std::chrono::seconds max_lifetime
        );

        /**
         * Get an existing session (if any) or create a new one.
         *
         * @param req The request.
         * @param resp The response.
         * @return The session.
         */
        [[nodiscard]]
        std::shared_ptr<session> start_session(const request& req, response& resp);

        /**
         * Destroys an existing session.
         *
         * @param req The request.
         * @param resp The response.
         */
        void destroy_session(const request& req, response& resp);

        /**
         * Destroys any sessions older than the configured max lifetime.
         *
         * @return The number of sessions that were expired/destroyed.
         */
        std::size_t destroy_expired_sessions();

    private:
        std::shared_ptr<storage> m_storage;
        std::chrono::seconds m_max_lifetime;
        std::mutex m_lock; // protects sessions
        std::string m_cookie_name = "sessionId";      // The name of the session cookie

        /**
         * Generates a new, unique session ID
         */
        [[nodiscard]]
        static id_type generate_session_id();
    };

}
