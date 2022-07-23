#pragma once

#include "../http/request.hpp"

#include <optional>
#include <string>
#include <vector>

namespace malloy::html
{

    /**
     * Class for representing a single field in a form.
     */
    struct form_field
    {
        struct parsed_data
        {
            std::string dispositions;
            std::string filename;
            std::string type;
            std::string content;
        };

        std::string name;                   /// The field name.
        std::string type;                   /// The HTML type (eg. 'text', 'file', ...).
        std::string value;                  /// The value of the field (for rendering only).
        std::string placeholder;            /// The placeholder.
        std::string label;                  /// The label content.
        bool required = false;              /// Whether a value is required.
        std::optional<parsed_data> data;    /// Data after parsing.

        /**
         * Populate the `value` member from the parsed data `content` member.
         *
         * @note This will ignore/skip if the `type` is "password".
         * @note This will ignore/skip if the `type` is "file".
         */
        void
        populate_value_from_parsed_data()
        {
            if (!data)
                return;

            if (type == "password" ||
                type == "file"
            )
                return;

            value = data->content;
        }

        [[nodiscard]]
        bool
        has_data() const
        {
            return data.has_value();
        }

        [[nodiscard]]
        std::string
        html_id() const
        {
            return "form-field-" + name;
        }
    };

    /**
     * Class for handling HTML forms.
     *
     * This class supports all three encoding types supported by HTML5:
     *   - application/x-www-form-urlencoded
     *   - multipart/form-data
     *   - text/plain
     */
    class form
    {
    public:
        enum class encoding
        {
            url,
            multipart,
            plain,
        };

        /**
         * Constructor.
         *
         * @param method The HTTP method.
         * @param action the Action (eg. target link).
         * @param enc The encoding type.
         */
        form(
            http::method method,
            std::string action,
            encoding enc
        );

        /**
         * Destructor.
         */
        virtual ~form() = default;

        /**
         * Get the method.
         *
         * @return The method.
         */
        [[nodiscard]]
        http::method
        method() const noexcept
        {
            return m_method;
        }

        /**
         * Get the action string.
         *
         * @return The action string.
         */
        [[nodiscard]]
        std::string_view
        action() const noexcept
        {
            return m_action;
        }

        [[nodiscard]]
        encoding
        get_encoding() const noexcept
        {
            return m_encoding;
        }

        /**
         * Returns the MIME type corresponding to the encoding type.
         *
         * @return The MIME type (eg. "application/x-www-form-urlencoded").
         */
        [[nodiscard]]
        std::string
        encoding_string() const;

        /**
         * Get the form fields.
         *
         * @return The form fields.
         */
        [[nodiscard]]
        std::vector<form_field>
        fields() const noexcept
        {
            return m_fields;
        }

        /**
         * Adds a field to the form.
         *
         * @note This will fail if the supplied field has an invalid name.
         * @note This will fail if a field with the same name exists already.
         *
         * @param field The field.
         * @return Whether the field was added.
         */
        bool
        add_field(form_field&& field);

        /**
         * Checks whether a field with a specific name already exists.
         *
         * @param field_name The field name.
         * @return Whether a field with the specified name already exists.
         */
        bool
        has_field(std::string_view field_name) const;

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
         * Get the parsed data of a specific field.
         *
         * @param field_name The name of the field.
         * @return The corresponding data (if any).
         */
        [[nodiscard]]
        std::optional<form_field::parsed_data>
        data(const std::string& field_name) const;

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
         * This will update each field's value member with the content member of
         * the parsed data (if any).
         *
         * This function is useful to call if serving the same form back to the user after
         * parsing to pre-populate form fields so the user doesn't have to re-enter everything.
         *
         * @note This will skip/ignore any fields of HTML type 'password'.
         * @note This will skip/ignore any fields of HTML type 'file'.
         */
        void
        populate_values_from_parsed_data();

        /**
         * Clears the pre-population values of all fields.
         */
        void
        clear_values();

        /**
         * Dumps the key-value pairs as a readable string.
         *
         * @return Key-value pairs represented as a string
         */
        [[nodiscard]]
        std::string
        dump() const;

        /**
         * Parse a request into this form.
         *
         * @param req The request.
         * @return Whether the parsing was successful.
         */
        bool
        parse(const malloy::http::request<>& req);

    private:
        http::method m_method;
        std::string m_action;
        encoding m_encoding;
        std::vector<form_field> m_fields;

        /**
         * @note This will throw if no field with `field_name` exists.
         *
         * @param field_name The field name.
         * @return The field.
         */
        [[nodiscard]]
        form_field&
        field_from_name(std::string_view field_name);

        /**
         * @note This will throw if no field with `field_name` exists.
         *
         * @param field_name The field name.
         * @return The field.
         */
        [[nodiscard]]
        const form_field&
        field_from_name(std::string_view field_name) const;

        [[nodiscard]]
        bool
        parse_urlencoded(const malloy::http::request<>& req);

        [[nodiscard]]
        bool
        parse_multipart(const malloy::http::request<>& req);

        [[nodiscard]]
        bool
        parse_plain(const malloy::http::request<>& req);
    };

}
