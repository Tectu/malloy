#pragma once

#include "types.hpp"
#include "../cookie.hpp"

#include <functional>
#include <string>
#include <unordered_map>
#include <optional>

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
    struct session
    {
        using key_type    = std::string;
        using value_type  = std::string;
        using id_type     = std::string;

        /**
         * Constructor.
         *
         * @note Sub-classes must call `update_access_time()` in their constructor.
         *
         * @param id The session ID.
         */
        explicit session(id_type&& id) :
            m_id(std::move(id))
        {
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
            if (key.empty() or value.empty())
                return false;

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
            if (key.empty())
                return std::nullopt;

            update_access_time();
            const auto& value_opt = storage_get(key);

            // Prevent returning empty values
            if (value_opt and value_opt.value().empty())
                return std::nullopt;

            return value_opt;
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
            if (key.empty())
                return false;

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

        /**
         * Update the access time.
         *
         * This should usually update the access time to the current time.
         */
        virtual
        void update_access_time() = 0;

    private:
        id_type m_id;
    };

    /**
     * A session implementing the access time using std::chrono.
     *
     * @tparam Clock The clock type to use.
     */
    template<typename Clock>
    struct session_chrono :
        session
    {
        using time_point_t = std::chrono::time_point<Clock>;

    public:
        /**
         * Constructor.
         *
         * @param id The session ID.
         */
        explicit session_chrono(id_type&& id) :
            session(std::move(id))
        {
            m_access_time = Clock::now();
        }

        /**
         * Get the access time.
         *
         * @return The access time.
         */
        time_point_t access_time() const noexcept
        {
            return m_access_time;
        }

        /**
         * Checks whether this session's access time is older than a specified maximum lifetime.
         *
         * @tparam Duration The duration type.
         * @param duration The ma
         * @return
         */
        template<typename Rep, typename Period>
        [[nodiscard]]
        constexpr bool access_time_older_than(const std::chrono::duration<Rep, Period>& max_lifetime) const
        {
            return (Clock::now() - m_access_time) > max_lifetime;
        }

    protected:
        /**
         * @copydoc session::update_access_time()
         */
        void update_access_time() override
        {
            m_access_time = Clock::now();
        }

    private:
        time_point_t m_access_time;
    };

}
