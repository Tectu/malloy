#include "../../test.hpp"
#include <malloy/core/http/session/storage_memory.hpp>

using namespace malloy::http::sessions;
using namespace std::chrono_literals;

TEST_SUITE("components - http - sessions - storage_memory")
{
    TEST_CASE("destroy expired")
    {
        storage_memory s;

        CHECK(s.create("0"));
        CHECK(s.create("1"));
        CHECK(s.create("2"));

        CHECK_EQ(s.destroy_expired(100s), 0);

        std::this_thread::sleep_for(2s);
        CHECK(s.create("3"));
        CHECK(s.create("4"));
        CHECK_EQ(s.destroy_expired(1s), 3);

        std::this_thread::sleep_for(2s);
        CHECK_EQ(s.destroy_expired(1s), 2);
    }
}
