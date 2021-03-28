#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include <iostream>

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
        uri(const uri& other) = default;

        /**
         * Move ctor.
         *
         * @param other The object to move-construct from.
         */
        uri(uri&& other) noexcept = default;

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
        uri& operator=(const uri& rhs) = default;

        /**
         * Move assignment operator.
         *
         * @param rhs The right-hand-side object to move-assign from.
         * @return A reference to left-hand-side object.
         */
        uri& operator=(uri&& rhs) noexcept = default;

        [[nodiscard]]
        std::string_view resource() const noexcept { return m_resource; }

        [[nodiscard]]
        std::string_view query_string() const noexcept { return m_query_string; }

        [[nodiscard]]
        auto query() const noexcept
        {
            return m_query;
        }

        [[nodiscard]]
        std::string_view fragment() const noexcept { return m_fragment; }

    private:
        std::string m_raw;
        std::string_view m_resource;
        std::string_view m_query_string;
        std::string_view m_fragment;
        std::unordered_map<std::string_view, std::string_view> m_query;

        void parse();
    };

}
