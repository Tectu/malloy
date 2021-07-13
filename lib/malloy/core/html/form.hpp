#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "malloy/core/http/request.hpp"

namespace malloy::html
{

    /**
     * Class for handling HTML forms.
     *
     * This class can be used to parse an HTML form.
     * Form data is represented as key-value pairs where both `key` and
     * `value` are of type `std::string`.
     */
    class form
    {
    public:
        form() = default;
        virtual ~form() = default;

        /**
         * Parse a request into this form.
         *
         * @param req The request.
         * @return Whether the parsing was successful.
         */
        bool parse(const malloy::http::request<>& req);

        /**
         * Checks whether a specific key-value pair is present.
         *
         * @param key The key.
         * @return Whether the key-value pair is present.
         */
        [[nodiscard]]
        bool has_value(const std::string& key) const;

        /**
         * Get the value associated with a specified key (if any).
         *
         * @param key The key.
         * @return The matching value (if any).
         */
        [[nodiscard]]
        std::optional<std::string> value(const std::string& key) const;

        /**
         * Get all key-value pairs.
         *
         * @return The key-value pairs.
         */
        [[nodiscard]]
        std::unordered_map<std::string, std::string> values() const noexcept
        {
            return m_values;
        }

    private:
        std::unordered_map<std::string, std::string> m_values;
    };

}
