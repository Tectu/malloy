#include "form_renderer.hpp"
#include "form.hpp"

#include <sstream>

using namespace malloy::html;

std::string
form_renderer_basic::render(const form& f) const
{
    std::stringstream ss;

    ss << "<form method=\"" << boost::beast::http::to_string(f.method()) << "\" action=\"" << f.action() << "\" enctype=\"" << f.encoding_to_string() << "\">\n";
    for (const form_field& field : f.fields()) {
        if (field.type == "submit") {
            ss << "  <button type=\"submit\">" + field.name + "</button>";
        }

        else {
            if (!field.label.empty())
                ss << "  <label for=\"" << field.html_id() << "\">" << field.label << "</label>\n";
            ss << "  <input id=\"" << field.html_id() << "\" type=\"" << field.type << "\" name=\"" + field.name + "\" placeholder=\"" << field.placeholder << "\" value=\"" << field.value << "\"/>\n";
        }
    }
    ss << "</form>";

    return ss.str();
}
