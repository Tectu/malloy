#include "../../test.hpp"

#include <malloy/core/html/multipart_parser.hpp>

using namespace malloy;

TEST_SUITE("components - html - multipart parser")
{
    auto check_part = [](
        const html::multipart_parser::part& p,
        const std::string_view& expected_disposition,
        const std::string_view& expected_type,
        const std::string_view& expected_content
    ){
        CHECK_EQ(p.disposition, expected_disposition);
        CHECK_EQ(p.type, expected_type);
        CHECK_EQ(p.content, expected_content);
    };

    TEST_CASE("regular 1")
    {
        constexpr const char* boundary{ "---------------------------735323031399963166993862150" };
        constexpr const char* body{
            "-----------------------------735323031399963166993862150\r\n"
            "Content-Disposition: form-data; name=\"text1\"\r\n"
            "\r\n"
            "text default\r\n"
            "-----------------------------735323031399963166993862150\r\n"
            "Content-Disposition: form-data; name=\"text2\"\r\n"
            "\r\n"
            "aωb\r\n"
            "-----------------------------735323031399963166993862150\r\n"
            "Content-Disposition: form-data; name=\"file1\"; filename=\"a.txt\"\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "Content of a.txt.\n"
            "\r\n"
            "-----------------------------735323031399963166993862150\r\n"
            "Content-Disposition: form-data; name=\"file2\"; filename=\"a.html\"\r\n"
            "Content-Type: text/html\r\n"
            "\r\n"
            "<!DOCTYPE html><title>Content of a.html.</title>"
            "\r\n"
            "-----------------------------735323031399963166993862150\r\n"
            "Content-Disposition: form-data; name=\"file3\"; filename=\"binary\"\r\n"
            "Content-Type: application/octet-stream\r\n"
            "\r\n"
            "aωb\n"
            "\r\n"
            "-----------------------------735323031399963166993862150--"
        };

        const auto& parts = html::multipart_parser::parse(body, boundary);
        REQUIRE_EQ(parts.size(), 5);
        check_part(
            parts[0],
            "form-data; name=\"text1\"",
            "",
            "text default"
        );
        check_part(
            parts[1],
            "form-data; name=\"text2\"",
            "",
            "aωb"
        );
        check_part(
            parts[2],
            "form-data; name=\"file1\"; filename=\"a.txt\"",
            "text/plain",
            "Content of a.txt.\n"
        );
        check_part(
            parts[3],
            "form-data; name=\"file2\"; filename=\"a.html\"",
            "text/html",
            "<!DOCTYPE html><title>Content of a.html.</title>"
        );
        check_part(
            parts[4],
            "form-data; name=\"file3\"; filename=\"binary\"",
            "application/octet-stream",
            "aωb\n"
        );
    }

    TEST_CASE("regular 2")
    {
        constexpr const char* boundary{ "---------------------------29923525376936588354272799774" };
        constexpr const char* body{
            "-----------------------------29923525376936588354272799774\r\n"
            "Content-Disposition: form-data; name=\"test1\"\r\n"
            "\r\n"
            "test1\r\n"
            "-----------------------------29923525376936588354272799774\r\n"
            "Content-Disposition: form-data; name=\"test2\"\r\n"
            "\r\n"
            "test2"
            "\r\n"
            "-----------------------------29923525376936588354272799774\r\n"
            "Content-Disposition: form-data; name=\"test3\"\r\n"
            "\r\n"
            "test3\n"
            "\r\n"
            "-----------------------------29923525376936588354272799774\r\n"
            "Content-Disposition: form-data; name=\"image\"; filename=\"foo.txt\"\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "Line 1\n"
            "Line 2\n"
            "Line 3\n"
            "\n"
            "\r\n"
            "-----------------------------29923525376936588354272799774--"
        };

        const auto& parts = html::multipart_parser::parse(body, boundary);
        REQUIRE_EQ(parts.size(), 4);
        check_part(
            parts[0],
            "form-data; name=\"test1\"",
            "",
            "test1"
        );
        check_part(
            parts[1],
            "form-data; name=\"test2\"",
            "",
            "test2"
        );
        check_part(
            parts[2],
            "form-data; name=\"test3\"",
            "",
            "test3\n"
        );
        check_part(
            parts[3],
            "form-data; name=\"image\"; filename=\"foo.txt\"",
            "text/plain",
            "Line 1\nLine 2\nLine 3\n\n"
        );
    }

}
