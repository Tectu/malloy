#include "form_renderer.hpp"

#include <malloy/core/html/form.hpp>

#include <sstream>

std::string
form_renderer::render(const malloy::html::form& f) const
{
    using namespace malloy::html;

    std::stringstream ss;

    ss << "<form class=\"w3-container\" method=\"" << boost::beast::http::to_string(f.method()) << "\" action=\"" << f.action() << "\" enctype=\"" << f.encoding_string() << "\">\n";
    for (const form_field& field : f.fields()) {
        if (field.type == "submit") {
            ss << "  <button class=\"w3-btn w3-padding w3-teal\" style=\"width:120px\" type=\"submit\">" + field.name + "</button>";
        }

        else {
            if (!field.label.empty())
                ss << "  <label for=\"" << field.html_id() << "\">" << field.label << "</label>\n";
            ss << "  <input id=\"" << field.html_id() << "\" class=\"w3-input\" type=\"" << field.type << "\" name=\"" + field.name + "\" placeholder=\"" << field.placeholder << "\" value=\"" << field.value << "\"/>\n";
        }
    }
    ss << "</form>";

    return ss.str();
}