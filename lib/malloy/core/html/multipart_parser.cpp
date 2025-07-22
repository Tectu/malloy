#include "multipart_parser.hpp"
#include "../utils.hpp"
#include "../http/utils.hpp"

using namespace malloy;
using namespace malloy::html;

std::vector<multipart_parser::part>
multipart_parser::parse(const malloy::http::request<>& req)
{
    using namespace std::literals;

    // Look for the boundary
    std::string boundary;
    {
        // Get Content-Type header
        const std::string_view& content_type = req.at(malloy::http::field::content_type);
        const auto &content_type_parts = malloy::http::split_header_value(content_type);

        // Look for boundary
        for (const auto& part : content_type_parts) {
            // Look for part containing boundary
            if (!part.starts_with("boundary"))
                continue;

            // Extract boundary
            const auto& boundary_pair = split(part, "="sv);
            if (boundary_pair.size() != 2)
                continue;

            // Assign
            boundary = boundary_pair[1];
        }
    }
    if (boundary.empty())
        return { };

    // Parse
    return parse(req.body(), boundary);
}

std::vector<multipart_parser::part>
multipart_parser::parse(const std::string_view body, const std::string& boundary)
{
    std::vector<part> parts;

    // Sanity check
    if (body.empty() || boundary.empty())
        return { };

    // Prepare boundary lines
    const std::string boundary_line      = "--" + boundary + "\r\n";
    const std::string boundary_line_last = "--" + boundary + "--";

    // Build list of boundary indexes
    std::vector<std::string_view::size_type> boundary_starts;
    std::string_view::size_type boundary_pos = 0;
    while (true) {
        // Look for boundary line
        boundary_pos = body.find(boundary_line, boundary_pos);
        if (boundary_pos == std::string_view::npos || boundary_pos >= body.size())
            break;

        boundary_starts.push_back(boundary_pos);

        // Move on to the next boundary
        boundary_pos += boundary_line.size();
    }
    if (boundary_starts.empty())
        return { };

    // Find the location of the ending boundary line
    const auto& last_boundary_pos = body.find(boundary_line_last, boundary_starts.back());
    if (last_boundary_pos == std::string_view::npos)
        return { };
    boundary_starts.push_back(last_boundary_pos);

    // Split into individual bodies/parts
    std::vector<std::string_view> parts_raw;
    for (std::size_t i = 0; i < boundary_starts.size()-1; ++i) {
        const auto& start_pos = boundary_starts[i + 0] + boundary_line.size();
        const auto& end_pos   = boundary_starts[i + 1];
        const auto& count     = end_pos - start_pos;

        // Extract the part/body
        const std::string_view& part_raw = body.substr(start_pos, count);

        // Parse the part/body
        const auto& part = parse_part(part_raw);
        if (part)
            parts.emplace_back(std::move(part.value()));
    }

    return parts;
}

std::optional<multipart_parser::part>
multipart_parser::parse_part(const std::string_view part_raw)
{
    struct part p;

    // Sanity check
    if (part_raw.empty())
        return { };

    // Line-by-line
    std::string_view::size_type char_counter = 0;
    const auto &lines = split(part_raw, "\r\n");
    for (const std::string_view &line : lines) {
        // Keep track of how many characters we already processed
        char_counter += line.size() + 2;    // +2 for \r\n which was stripped in the split

        // Get Content-Disposition
        if (line.starts_with(str_disposition)) {
            p.disposition = line.substr(str_disposition.size());
            continue;
        }

        // Get Content-Type
        if (line.starts_with(str_type)) {
            p.type = line.substr(str_type.size());
            continue;
        }

        // Look for empty line before content starts
        if (line.empty())
            break;
    }

    // Grab the content
    p.content = part_raw.substr(char_counter, part_raw.size()-char_counter-2);  // We do -2 to drop the last \r\n after the value

    return p;
}
