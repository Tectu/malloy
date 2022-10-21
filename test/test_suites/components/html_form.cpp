#include "../../test.hpp"

#include <malloy/core/html/form.hpp>

using namespace malloy;

TEST_SUITE("components - html - form")
{

    TEST_CASE("encodings")
    {
        using namespace malloy::html;

        SUBCASE("to string")
        {
            CHECK_EQ(form::encoding_to_string(form::encoding::url),         "application/x-www-form-urlencoded");
            CHECK_EQ(form::encoding_to_string(form::encoding::multipart),   "multipart/form-data");
            CHECK_EQ(form::encoding_to_string(form::encoding::plain),       "text/plain");
        }

        SUBCASE("from string")
        {
            // Valid
            CHECK_EQ(form::encoding_from_string("application/x-www-form-urlencoded"),   form::encoding::url);
            CHECK_EQ(form::encoding_from_string("multipart/form-data"),                 form::encoding::multipart);
            CHECK_EQ(form::encoding_from_string("text/plain"),                          form::encoding::plain);

            // Invalid
            CHECK_EQ(form::encoding_from_string(""),    std::nullopt);
            CHECK_EQ(form::encoding_from_string("abc"), std::nullopt);
        }
    }

}
