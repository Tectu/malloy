#pragma once

#include "types.hpp"
#include "malloy/core/http/response.hpp"
#include "malloy/core/http/request.hpp"

#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

namespace malloy::http::sessions
{
    struct storage;
    struct session;

    class manager
    {
    public:
        /**
         * Name of the session cookie
         */
        constexpr static const char* cookie_name = "sessionId";

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
         * Get an existing session (if any).
         *
         * @param req The HTTP request.
         * @return The session (if any).
         *
         * @sa start()
         */
        [[nodiscard]]
        std::shared_ptr<session> get(const request<>& req);

        /**
         * Get an existing session (if any) or create a new one.
         *
         * @param req The request.
         * @param resp The response.
         * @return The session.
         *
         * @sa get()
         * @sa delete()
         */
        [[nodiscard]]
        std::shared_ptr<session> start(const request<>& req, response<>& resp);

        /**
         * Destroys an existing session.
         *
         * @param req The request.
         * @param resp The response.
         *
         * @sa start()
         */
        void destroy(const request<>& req, response<>& resp);

        /**
         * Destroys any sessions older than the specified max lifetime.
         *
         * @param max_lifetime The maximum lifetime of a session.
         * @return The number of sessions that were expired/destroyed.
         */
        std::size_t destroy_expired(const std::chrono::seconds& max_lifetime);

        /**
         * Checks whether the session is valid.
         *
         * @param req The request.
         * @return Whether the session is valid.
         */
        [[nodiscard]]
        bool is_valid(const request<>& req);

    private:
        std::shared_ptr<storage> m_storage;
        std::mutex m_lock; // protects sessions

        /**
         * Gets the session ID from a request (if any).
         *
         * @param req The request.
         * @return The session ID (if any).
         */
        [[nodiscard]]
        std::optional<id_type> get_id(const request<>& req) const;

        /**
         * Generates a new, unique session ID
         */
        [[nodiscard]]
        static id_type generate_id();
    };

}
