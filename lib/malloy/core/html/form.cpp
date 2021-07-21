#include "form.hpp"
#include "multipart_parser.hpp"
#include "../utils.hpp"
#include "../http/utils.hpp"

#include <algorithm>
#include <vector>

using namespace malloy::html;

form::form(
    const http::method method,
    std::string action,
    const encoding enc
) :
    m_method(method),
    m_action(std::move(action)),
    m_encoding(enc)
{
}

form_field&
form::field_from_name(const std::string_view field_name)
{
    const auto& it = std::find_if(
        std::begin(m_fields),
        std::end(m_fields),
        [&field_name](const auto& ef) {
            return ef.name == field_name;
        }
    );
    if (it == std::end(m_fields))
        throw std::logic_error("field with specified name does not exists.");

    return *it;
}

const form_field&
form::field_from_name(const std::string_view field_name) const
{
    const auto& it = std::find_if(
        std::begin(m_fields),
        std::end(m_fields),
        [&field_name](const auto& ef) {
            return ef.name == field_name;
        }
    );
    if (it == std::end(m_fields))
        throw std::logic_error("field with specified name does not exists.");

    return *it;
}

// ToDo: This should be handled by malloy's core MIME-type system.
std::string
form::encoding_string() const
{
    switch (m_encoding) {
        case encoding::url:         return "application/x-www-form-urlencoded";
        case encoding::multipart:   return "multipart/form-data";
        case encoding::plain:       return "text/plain";
    }

    return { };
}

bool
form::add_field(form_field&& field)
{
    // Sanity check name
    if (field.name.empty())
        return false;

    // Prevent adding a field with the same name
    if (has_field(field.name))
        return false;

    m_fields.emplace_back(std::move(field));

    return true;
}

bool
form::has_field(const std::string_view field_name) const
{
    const auto& it = std::find_if(
        std::cbegin(m_fields),
        std::cend(m_fields),
        [&field_name](const auto& ef) {
            return ef.name == field_name;
        }
    );

    return it != std::cend(m_fields);
}

bool
form::has_data(const std::string_view field_name) const
{
    const auto& it = std::find_if(
        std::cbegin(m_fields),
        std::cend(m_fields),
        [&field_name](const auto& ef) {
            return ef.name == field_name;
        }
    );
    if (it == std::cend(m_fields))
        return false;

    return it->has_data();
}

std::optional<form_field::parsed_data>
form::data(const std::string& field_name) const
{
    // Check whether field exists
    if (!has_field(field_name))
        return std::nullopt;

    // Retrieve field
    const auto& it = std::find_if(
        std::cbegin(m_fields),
        std::cend(m_fields),
        [&field_name](const auto& ef) {
            return ef.name == field_name;
        }
    );
    if (it == std::cend(m_fields))
        return std::nullopt;

    // Return field value
    return it->data;
}

bool
form::has_content(const std::string_view field_name) const
{
    // Check whether the field exists
    if (!has_field(field_name))
        return false;

    // Retrieve field
    const auto& field = field_from_name(field_name);

    // Check if field has parsed data
    if (!field.has_data())
        return false;

    // Check if field data has content
    return !field.data->content.empty();
}

std::optional<std::string>
form::content(const std::string_view field_name) const
{
    // Check whether the field exists
    if (!has_field(field_name))
        return std::nullopt;

    // Retrieve field
    const auto& field = field_from_name(field_name);

    // Check if field has parsed data
    if (!field.has_data())
        return std::nullopt;

    // Return content
    return field.data->content;
}

void
form::populate_values_from_parsed_data()
{
    for (auto& field : m_fields)
        field.populate_value_from_parsed_data();
}

void
form::clear_values()
{
    for (auto& field : m_fields)
        field.value = { };
}

std::string
form::dump() const
{
    std::ostringstream ss;

    for (const form_field& field : m_fields) {
        if (!field.has_data())
            continue;

        ss << field.name << ":\n";
        ss << "  type     = " << field.data->type << "\n";
        ss << "  filename = " << field.data->filename << "\n";
        ss << "  content  = " << field.data->content << "\n";
        ss << "\n";
    }

    return ss.str();
}

bool
form::parse(const malloy::http::request<>& req)
{
    switch (m_encoding) {
        case encoding::url:         return parse_urlencoded(req);
        case encoding::multipart:   return parse_multipart(req);
        case encoding::plain:       return parse_plain(req);
    }

    return false;
}

bool
form::parse_urlencoded(const malloy::http::request<>& req)
{
    using namespace std::literals;

    // Sanity check encoding type
    if (m_encoding != encoding::url)
        throw std::logic_error("form encoding type does not match");

    try {
        // Split pairs
        const std::vector<std::string_view> pairs = split(req.body(), "&"sv);
        for (const std::string_view& pair_str : pairs) {
            const std::vector<std::string_view> pair = split(pair_str, "="sv);
            if (pair.size() != 2)
                continue;

            // Perform URL decoding
            std::string value{ pair[1] };
            url_decode(value);

            // Find field
            form_field& field = field_from_name(pair[0]);

            // Store value
            form_field::parsed_data data;
            data.content = std::move(value);
            field.data = std::move(data);
        }

        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool
form::parse_multipart(const malloy::http::request<>& req)
{
    using namespace std::literals;

    // Sanity check encoding type
    if (m_encoding != encoding::multipart)
        throw std::logic_error("form encoding type does not match");

    try {
        // Parse
        const auto& parts = multipart_parser::parse(req);
        for (const multipart_parser::part& part : parts) {
            // Decompose disposition
            std::string locator_name{ "name=" };
            std::string locator_filename{ "filename=" };
            std::string_view name;
            std::string_view filename;
            const auto& disposition_parts = http::split_header_value(part.disposition);
            for (const std::string_view& dp : disposition_parts) {
                // Name
                if (dp.starts_with(locator_name)) {
                    name = dp.substr(locator_name.size()+1, dp.size()-locator_name.size()-2);   // Drop the surrounding quotation marks
                    continue;
                }

                // Filename
                if (dp.starts_with(locator_filename)) {
                    filename = dp.substr(locator_filename.size()+1, dp.size()-locator_filename.size()-2);   // Drop surrounding quotation marks
                    continue;
                }
            }
            if (name.empty())
                return false;  // According to the 'multipart/form-data' spec the 'name' field is required

            // Look for corresponding field
            if (!has_field(name))
                continue;
            form_field& field = field_from_name(name);

            // Fill field
            form_field::parsed_data data;
            data.dispositions = part.disposition;
            data.filename = filename;
            data.type = part.type;
            data.content = part.content;
            field.data = std::move(data);
        }
    }
    catch (const std::exception&) {
        return false;
    }

    return true;
}

bool
form::parse_plain(const malloy::http::request<>& req)
{
    using namespace std::literals;

    // Sanity check encoding type
    if (m_encoding != encoding::plain)
        throw std::logic_error("form encoding type does not match");

    try {
        // Each field is encoded as: {key}={value}\r\n
        for (const std::string_view& line : split(req.body(), "\r\n"sv)) {
            // Retrieve key/value
            const auto& pair = split(line, "="sv);
            if (pair.size() != 2)
                continue;
            const std::string_view& name = pair[0];
            const std::string_view& value = pair[1];

            // Find corresponding field
            form_field& field = field_from_name(name);

            // Store content
            form_field::parsed_data data;
            data.content = value;
            field.data = std::move(data);
        }
    }
    catch (const std::exception&) {
        return false;
    }

    return true;
}
