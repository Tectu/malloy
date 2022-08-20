#pragma once

#include "form_data.hpp"
#include "form_field.hpp"
#include "../http/request.hpp"

#include <optional>
#include <string>
#include <vector>

namespace malloy::html
{

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
        /**
         * Encoding type.
         */
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
        virtual
        ~form() = default;

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

        /**
         * Get encoding type.
         *
         * @return The encoding type.
         */
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
         * This will update each field's value member with the content member of
         * the parsed data;
         *
         * This function is useful to call if serving the same form back to the user after
         * parsing to pre-populate form fields so the user doesn't have to re-enter everything.
         *
         * @note This will skip/ignore any fields of HTML type 'password'.
         * @note This will skip/ignore any fields of HTML type 'file'.
         *
         * @param data The data.
         */
        void
        populate(const form_data& data);

        /**
         * Clears the pre-population values of all fields.
         */
        void
        clear_values();

        /**
         * Parse a request matching this form.
         *
         * @param req The request.
         * @return The parsed form data (if any).
         */
        std::optional<form_data>
        parse(const malloy::http::request<>& req) const;

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
        std::optional<form_data>
        parse_urlencoded(const malloy::http::request<>& req) const;

        [[nodiscard]]
        std::optional<form_data>
        parse_multipart(const malloy::http::request<>& req) const;

        [[nodiscard]]
        std::optional<form_data>
        parse_plain(const malloy::http::request<>& req) const;
    };

}
