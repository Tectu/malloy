#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace malloy::http
{

    /**
     * Represents an URI.
     */
    class uri
    {
    public:
        /**
         * Default ctor.
         */
        uri() = default;

        /**
         * Construct an URI from a string.
         *
         * @param str The string to parse.
         */
        explicit uri(std::string&& str) :
            m_raw(std::move(str))
        {
            parse();
        }

        /**
         * Copy ctor.
         *
         * @param other The object to copy-construct from.
         */
        uri(const uri& other) :
            m_raw(other.m_raw)
        {
            parse();
        }

        /**
         * Move ctor.
         *
         * @param other The object to move-construct from.
         */
        uri(uri&& other) :
            m_raw(std::move(other.m_raw))
        {
            parse();
        }

        /**
         * Destructor.
         */
        virtual ~uri() = default;

        /**
         * Copy assignment operator.
         *
         * @param rhs The right-hand-side object to copy-assign from.
         * @return A reference to the left-hand-side object.
         */
        uri& operator=(const uri& rhs)
        {
            m_raw = rhs.m_raw;

            parse();

            return *this;
        }

        /**
         * Move assignment operator.
         *
         * @param rhs The right-hand-side object to move-assign from.
         * @return A reference to left-hand-side object.
         */
        uri& operator=(uri&& rhs)
        {
            m_raw = std::move(rhs.m_raw);

            parse();

            return *this;
        }

        /**
         * Checks whether the URI is legal.
         *
         * To be considered legal, ALL of the following requirements must be true:
         * @li The resource path must not be empty
         * @li The resource path must start with "/"
         * @li The resource path must not contains ".."
         *
         * @return Whether this URI is considered to be legal.
         */
        [[nodiscard]]
        bool is_legal() const;

        /**
         * Retrieve the raw, non-parsed string.
         *
         * @return The raw, non-parsed string.
         */
        [[nodiscard]]
        std::string_view raw() const noexcept { return m_raw; }

        /**
         * Retrieve the resource path portion of the URI as a string.
         *
         * @return The resource path portion.
         */
        [[nodiscard]]
        std::string_view resource_string() const noexcept { return m_resource_string; }

        /**
         * Retrieve the resource path as separated strings.
         *
         * @return The resource path.
         */
        [[nodiscard]]
        std::vector<std::string_view> resource() const noexcept { return m_resource; }

        /**
         * Checks whether the resource path starts with a given string.
         *
         * @param str The string to check.
         * @return Whether the resource portion of the URI starts with provided string.
         */
        [[nodiscard]]
        bool resource_starts_with(std::string_view str) const
        {
            return m_resource_string.starts_with(str);
        }

        /**
         * Chops of a parts of the resource path portion.
         *
         * @param str The portion to chop off.
         * @return `true` on success, `false` otherwise.
         */
        bool chop_resource(std::string_view str);

        /**
         * Retrieve the query portion of the URI.
         *
         * @return The query portion of the URI.
         */
        [[nodiscard]]
        std::string_view query_string() const noexcept { return m_query_string; }

        /**
         * Retrieve the parsed query.
         *
         * @return Key-Value map of the query.
         */
        [[nodiscard]]
        auto query() const noexcept
        {
            return m_query;
        }

        /**
         * Retrieve the fragment portion of the URI.
         *
         * @return The fragment.
         */
        [[nodiscard]]
        std::string_view fragment() const noexcept { return m_fragment; }

        /**
         * Print this URI in it's parsed form.
         *
         * @return The printed string.
         */
        [[nodiscard]]
        std::string to_string() const;

    private:
        std::string m_raw;
        std::string_view m_resource_string;
        std::string_view m_query_string;
        std::string_view m_fragment;
        std::vector<std::string_view> m_resource;
        std::unordered_map<std::string_view, std::string_view> m_query;

        void parse();
        void parse_resource();
        void parse_query();
    };

}
