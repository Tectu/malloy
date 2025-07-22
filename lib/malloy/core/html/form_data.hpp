#pragma once

#include "form_field.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace malloy::html
{

    /**
     * Type to represent the data held by an HTML form.
     *
     * @sa form
     */
    class form_data
    {
    public:
        /**
         * The data for each field.
         */
        std::vector<form_field_data> fields;

        /**
         * Get a field by name.
         *
         * @param field_name The field name.
         * @return The field (if any).
         */
        [[nodiscard]]
        std::optional<form_field_data>
        field_by_name(std::string_view field_name) const;

        /**
         * Checks whether a field has parsed data.
         *
         * @param field_name The name of the field.
         * @return Whether the field has parsed data.
         */
        [[nodiscard]]
        bool
        has_data(std::string_view field_name) const;

        /**
         * Checks whether a particular field has parsed content.
         *
         * @param field_name The field name.
         * @return Whether the field has parsed content.
         */
        [[nodiscard]]
        bool
        has_content(std::string_view field_name) const;

        /**
         * The the parsed data content of a specific field.
         *
         * @param field_name The name of the field.
         * @return
         */
        [[nodiscard]]
        std::optional<std::string>
        content(std::string_view field_name) const;

        /**
         * Dumps the key-value pairs as a readable string.
         *
         * @return Key-value pairs represented as a string
         */
        [[nodiscard]]
        std::string
        dump() const;
    };

}
