#include <malloy/core/html/page.hpp>

#include <sstream>

class page1 :
    public malloy::html::page
{
public:
    [[nodiscard]]
    std::string
    render() const override
    {
        std::ostringstream ss;

        ss << "<html>\n";
        ss << "  <body>\n";
        ss << "    <h1>Page 1!</h1>\n";
        ss << "  </body>\n";
        ss << "</html>\n";

        return ss.str();
    }
};