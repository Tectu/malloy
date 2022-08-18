#pragma once

#include <string>

namespace malloy::html
{
    /**
     * Type to represent the data held by a single field of an HTML form.
     */
    struct form_field_data
    {
        std::string name; /// The field name.

        std::string dispositions;
        std::string filename;
        std::string type;
        std::string content;

        [[nodiscard]]
        bool
        has_data() const
        {
            return !content.empty();
        }
    };

    /**
     * Type to representing a single field of an HTML form.
     */
    struct form_field
    {
        std::string name;                   /// The field name.
        std::string type;                   /// The HTML type (eg. 'text', 'file', ...).
        std::string value;                  /// The value of the field (for rendering only).
        std::string placeholder;            /// The placeholder.
        std::string label;                  /// The label content.
        bool required = false;              /// Whether a value is required.

        /**
         * Populate the `value` member from the parsed data `content` member.
         *
         * @note This will ignore/skip if the `type` is "password".
         * @note This will ignore/skip if the `type` is "file".
         */
        void
        populate_value_from_parsed_data(const form_field_data& data)
        {
            if (data.type == "password" || data.type == "file")
                return;

            value = data.content;
        }

        /**
         * Get the HTML id.
         *
         * @return The HTML id.
         */
        [[nodiscard]]
        std::string
        html_id() const
        {
            return "form-field-" + name;
        }
    };

}
