#include "../../test.hpp"

#include <malloy/core/http/generator.hpp>

#include <filesystem>
#include <fstream>

using namespace malloy::http;

TEST_SUITE("components - http - generator")
{

    TEST_CASE("built-in responses")
    {
        SUBCASE("400 - bad request")
        {
            auto r = generator::bad_request("foobar");

            CHECK_EQ(r.status(), status::bad_request);
            CHECK_EQ(static_cast<int>(r.status()), 400);
            CHECK_EQ(r.body(), "foobar");
        }

        SUBCASE("404 - not found")
        {
            auto r = generator::not_found("foobar");

            CHECK_EQ(r.status(), status::not_found);
            CHECK_EQ(static_cast<int>(r.status()), 404);
            CHECK_EQ(r.body(), "The resource 'foobar' was not found.");
        }

        SUBCASE("500 - server error")
        {
            auto r = generator::server_error("foobar");

            CHECK_EQ(r.status(), status::internal_server_error);
            CHECK_EQ(static_cast<int>(r.status()), 500);
            CHECK_EQ(r.body(), "An error occurred: 'foobar'");
        }
    }

    TEST_CASE("file")
    {
        // Create a storage base path containing a single file
        const auto tmp = std::filesystem::temp_directory_path() / "malloy_test_http_generator_file";
        const auto base = tmp / "base";
        std::filesystem::remove_all(tmp);
        std::filesystem::create_directories(base);
        std::ofstream{ base / "index.html" } << "inside";
        std::ofstream{ tmp / "secret.txt" } << "outside";

        SUBCASE("serves file inside storage base path")
        {
            auto r = generator::file(base, "/index.html");

            REQUIRE(std::holds_alternative<response<boost::beast::http::file_body>>(r));
            CHECK_EQ(std::get<response<boost::beast::http::file_body>>(r).status(), status::ok);
        }

        SUBCASE("rejects relative path traversal")
        {
            auto r = generator::file(base, "/../secret.txt");

            REQUIRE(std::holds_alternative<response<boost::beast::http::string_body>>(r));
            CHECK_EQ(std::get<response<boost::beast::http::string_body>>(r).status(), status::bad_request);
        }

        SUBCASE("rejects absolute path escaping storage base path")
        {
            for (const auto& p : { "//", "///" }) {
                auto r = generator::file(base, p + (tmp / "secret.txt").relative_path().generic_string());

                REQUIRE(std::holds_alternative<response<boost::beast::http::string_body>>(r));
                CHECK_EQ(std::get<response<boost::beast::http::string_body>>(r).status(), status::not_found);
            }
        }

        std::filesystem::remove_all(tmp);
    }

}
