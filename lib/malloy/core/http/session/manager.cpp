#include "manager.hpp"
#include "session.hpp"
#include "storage.hpp"
#include "../utils.hpp"

#include <stdexcept>
#include <random>

using namespace malloy::http;
using namespace malloy::http::sessions;

manager::manager(std::shared_ptr<storage> storage) :
    m_storage(std::move(storage))
{
    if (!m_storage)
        throw std::invalid_argument("no valid storage provided.");
}

std::shared_ptr<session>
manager::get(const request_header<>& hdr)
{
    // Sanity check
    if (!m_storage)
        return {};

    // Get session ID
    const auto& ses_id = get_id(hdr);
    if (!ses_id.has_value())
        return {};

    // Get the session
    return get(ses_id.value());
}

std::shared_ptr<session>
manager::get(const id_type& ses_id)
{
    // Acquire mutex
    std::lock_guard lock(m_lock);

    // Check if storage has a session for this ID
    return m_storage->get(ses_id);
}

std::shared_ptr<session>
manager::start(const request<>& req, response<>& resp)
{
    // Nothing to do if no storage was provided
    if (!m_storage)
        return { };

    // Acquire the mutex
    std::lock_guard lock(m_lock);

    std::shared_ptr<session> session;

    // Get existing session (if any)
    const auto& ses_id = get_id(req);
    if (ses_id.has_value())
        session = m_storage->get(ses_id.value());

    // Otherwise create a new one
    if (!session) {
        // Generate a new session ID
        const id_type id = generate_id();

        // Create a new session
        session = m_storage->create(id);

        // Set the cookie
        // Only do this after creating a new session as we don't want to send back the
        // session cookie on every response (as subsequent calls to this function will
        // pass this branch
        if (session)
            resp.add_cookie(session->generate_cookie(cookie_name));
    }

    return session;
}

void
manager::destroy(const request<>& req, response<>& resp)
{
    if (!m_storage)
        return;

    // Get session ID
    const auto& ses_id = get_id(req);
    if (!ses_id.has_value())
        return;

    // Acquire mutex
    std::lock_guard lock(m_lock);

    // Destroy session in storage
    m_storage->destroy(std::string{ ses_id.value() });

    // Send back an invalid/expired cookie
    cookie_clear c(cookie_name);
    resp.add_cookie(c);
}

std::size_t
manager::destroy_expired(const std::chrono::seconds& max_lifetime)
{
    if (!m_storage)
        return 0;

    // Make sure that storage::destroy_expired_sessions() doesn't get a maximum
    // lifetime smaller than 1.
    if (max_lifetime <= std::chrono::seconds::zero())
        return 0;

    // Acquire mutex
    std::lock_guard lock(m_lock);

    return m_storage->destroy_expired(max_lifetime);
}

bool
manager::is_valid(const malloy::http::request_header<>& hdr)
{
    return get(hdr) != nullptr;
}

bool
manager::is_valid(const id_type& id)
{
    return get(id) != nullptr;
}

std::optional<id_type>
manager::get_id(const request_header<>& hdr) const
{
    // Get session cookie value
    const auto& ses_id = cookie_value(hdr, cookie_name);
    if (!ses_id)
        return std::nullopt;

    return std::string{ ses_id.value() };
}

id_type
manager::generate_id()
{
    /**
     * ToDo: This implementation is not good enough for production use!
     *       Also make this become an interface to allow the user to feed various entropy.
     *       Maybe that should be a higher-level malloy feature. Eg. maybe through the controller?
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
