#pragma once

#include "types.hpp"
#include "../response.hpp"
#include "../request.hpp"

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
         * @param hdr The HTTP request header.
         * @return The session (if any).
         *
         * @sa start()
         */
        [[nodiscard]]
        std::shared_ptr<session> get(const request_header<>& hdr);

        /**
         * Get an existing session (if any).
         *
         * @param req The HTTP request.
         * @return The session (if any).
         *
         * @sa start()
         */
        [[nodiscard]]
        std::shared_ptr<session> get(const request<>& req)
        {
            return get(req.base());
        }

        /**
         * Get an existing session (if any).
         *
         * @param id The session ID.
         * @return The session (if any).
         *
         * @sa start()
         */
        [[nodiscard]]
        std::shared_ptr<session> get(const id_type& id);

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
        bool is_valid(const request<>& req)
        {
            return is_valid(req.base());
        }

        /**
         * Checks whether the session is valid.
         *
         * @param hdr The request header.
         * @return Whether the session is valid.
         */
        [[nodiscard]]
        bool is_valid(const malloy::http::request_header<>& hdr);

        /**
         * Checks whether the session is valid.
         *
         * @param id The session id.
         * @return Whether the session is valid.
         */
        [[nodiscard]]
        bool is_valid(const id_type& id);

    private:
        std::shared_ptr<storage> m_storage;
        std::mutex m_lock; // protects sessions

        /**
         * Gets the session ID from a request (if any).
         *
         * @param req The request header.
         * @return The session ID (if any).
         */
        [[nodiscard]]
        std::optional<id_type> get_id(const request_header<>& hdr) const;

        /**
         * Generates a new, unique session ID
         */
        [[nodiscard]]
        static id_type generate_id();
    };

}
