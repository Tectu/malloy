#include <malloy/core/html/page.hpp>

#include <sstream>

namespace apps::gallery::pages
{

    class overview :
        public malloy::html::page
    {
    public:
        [[nodiscard]]
        std::string
        render() const override
        {
            std::ostringstream ss;

            ss << "<html>\n";
            ss << "  <head>\n";
            ss << "    <link rel=\"stylesheet\" href=\"./assets/style.css\"\n";
            ss << "  </head>\n";
            ss << "  <body>\n";
            ss << "    <h1>Gallery - Overview</h1>\n";
            ss << "    <div>\n";
            ss << "      <a href=\"http://127.0.0.1:8080/gallery/upload\">Upload</a>\n";
            ss << "    </div>\n";
            ss << "  </body>\n";
            ss << "</html>\n";

            return ss.str();
        }
    };

}
