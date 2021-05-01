#pragma once

#include "types.hpp"
#include "../cookie.hpp"

#include <functional>
#include <string>
#include <unordered_map>

namespace malloy::http::sessions
{

    /**
     * This class represents a session.
     *
     * This is a classic implementation for achieving a stateful session over stateless HTTP.
     * It works by generating a server-side session using an object of this class and sending the
     * session ID to the client as a cookie. The client will send back this cookie on sub-sequent
     * requests through which the server can retrieve the corresponding session ID.
     */
    class session
    {
    private:
        using clock_type      = std::chrono::steady_clock;
        using time_point_type = std::chrono::time_point<clock_type>;

    public:
        using key_type    = std::string;
        using value_type  = std::string;
        using id_type     = std::string;

        /**
         * Constructor.
         *
         * @param id The session ID.
         */
        explicit session(id_type&& id) :
            m_id(std::move(id))
        {
            update_access_time();
        }

        /**
         * Copy constructor.
         *
         * @param other
         */
        session(const session& other) = delete;

        /**
         * Move constructor
         *
         * @param other
         */
        session(session&& other) noexcept = delete;

        /**
         * Copy assignment operator.
         *
         * @param other
         * @return
         */
        session& operator=(const session& other) = delete;

        /**
         * Move assignment operator.
         *
         * @param other
         * @return
         */
        session& operator=(session&& other) noexcept = delete;

        /**
         * Add or update a key-value pair.
         *
         * If the specified key does not exist yet, a new key-value pair will be created.
         * If the key exists, the current value will be updated.
         *
         * @param key The key.
         * @param value The value.
         * @return Whether setting the value was successful.
         */
        bool set(const key_type& key, value_type value)
        {
            update_access_time();
            return storage_set(key, std::move(value));
        }

        /**
         * Get the value of a particular key.
         *
         * @param key The key.
         * @return The value corresponding to the specified key (if any).
         */
        [[nodiscard]]
        std::optional<key_type> get(const key_type& key)
        {
            update_access_time();
            return storage_get(key);
        }

        /**
         * Remove a key-value pair.
         *
         * @param key The key.
         * @return Whether a key-value pair was removed.
         */
        [[nodiscard]]
        virtual
        bool remove(const key_type& key)
        {
            update_access_time();
            return storage_remove(key);
        }

        /**
         * Get the session ID.
         *
         * @return The session ID.
         */
        [[nodiscard]]
        id_type id() const noexcept
        {
            return m_id;
        }

        /**
         * Generates a session cookie for this session.
         *
         * @param cookie_name The cookie name.
         * @return The session cookie.
         */
        [[nodiscard]]
        cookie generate_cookie(std::string cookie_name) const
        {
            return cookie {
                .name = std::move(cookie_name),
                .value = m_id,
                .max_age = { },
                .secure = true,
                .http_only = true,
                .same_site = cookie::same_site_t::strict,
            };
        }

    protected:
        /**
         * Add or update a key-value pair.
         *
         * If the specified key does not exist yet, a new key-value pair will be created.
         * If the key exists, the current value will be updated.
         *
         * @param key The key.
         * @param value The value.
         * @return Whether setting the value was successful.
         */
        virtual
        bool storage_set(const key_type& key, value_type value) = 0;

        /**
         * Get the value of a particular key.
         *
         * @param key The key.
         * @return The value corresponding to the specified key (if any).
         */
        [[nodiscard]]
        virtual
        std::optional<key_type> storage_get(const key_type& key) const = 0;

        /**
         * Remove a key-value pair.
         *
         * @param key The key.
         * @return Whether a key-value pair was removed.
         */
        [[nodiscard]]
        virtual
        bool storage_remove(const key_type& key) = 0;

    private:
        id_type m_id;
        time_point_type m_access_time;

        void update_access_time()
        {
            m_access_time = clock_type::now();
        }
    };

}
