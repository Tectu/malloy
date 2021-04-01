#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace malloy::http
{

    class uri
    {
    public:
        /**
         * Default ctor.
         */
        uri() = default;

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

        [[nodiscard]]
        bool is_legal() const;

        [[nodiscard]]
        std::string_view raw() const noexcept { return m_raw; }

        [[nodiscard]]
        std::string_view resource_string() const noexcept { return m_resource_string; }

        [[nodiscard]]
        std::vector<std::string_view> resource() const noexcept { return m_resource; }

        [[nodiscard]]
        bool resource_starts_with(std::string_view str) const
        {
            return m_resource_string.starts_with(str);
        }

        bool chop_resource(std::string_view str);

        [[nodiscard]]
        std::string_view query_string() const noexcept { return m_query_string; }

        [[nodiscard]]
        auto query() const noexcept
        {
            return m_query;
        }

        [[nodiscard]]
        std::string_view fragment() const noexcept { return m_fragment; }

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
