#include "manager.hpp"
#include "session.hpp"
#include "storage.hpp"
#include "../cookie.hpp"
#include "../request.hpp"
#include "../response.hpp"

#include <random>

using namespace malloy::http;
using namespace malloy::http::sessions;

bool manager::init(std::shared_ptr<storage> storage, std::chrono::seconds max_lifetime)
{
    // Sanity check storage
    if (not storage)
        return false;

    // Housekeeping
    m_storage = std::move(storage);
    m_max_lifetime = max_lifetime;

    return true;
}

std::shared_ptr<session> manager::start_session(const request& req, response& resp)
{
    // Nothing to do if no storage was provided
    if (not m_storage)
        return { };

    // Acquire the mutex
    std::lock_guard lock(m_lock);

    std::shared_ptr<session> session;

    // Get existing session (if any)
    if (req.has_cookie(m_cookie_name)) {
        const id_type id { req.cookie(m_cookie_name) };
        session = m_storage->get_session(id);
    }

    // Otherwise create a new one
    if (not session) {
        // Generate a new session ID
        const id_type id = generate_session_id();

        // Create a new session
        session = m_storage->create_session(id);

        // Set the cookie
        resp.add_cookie(session->generate_cookie(m_cookie_name));
    }

    return session;
}

id_type manager::generate_session_id()
{
    /**
     * ToDo: This implementation is not good enough for production use!
     */

    static constexpr std::size_t length = 32;

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist('a', 'z');

    std::string result;
    result.reserve(length);
    std::generate_n(
        std::back_inserter(result),
        length,
        [&dist, &mt]{
            return dist(mt);
        }
    );

    return result;
}
