#pragma once

#include "../../core/rest/concepts.hpp"

namespace malloy::server::rest::store
{

    /**
     * An in-memory object storage.
     *
     * @details An in-memory object storage based on `std::vector<>`.
     * @note Mainly useful for development.
     *
     * @tparam Object
     */
    template<object Object>
    struct in_memory
    {
        [[nodiscard]]
        Object
        add()
        {
            std::lock_guard lock(m_mtx);

            Object obj;
            obj.id = generate_id();

            m_storage.emplace_back(obj);

            return obj;
        }

        [[nodiscard]]
        std::optional<Object>
        get(const std::size_t id)
        {
            std::lock_guard lock(m_mtx);

            const auto& it = std::find_if(
                std::cbegin(m_storage),
                std::cend(m_storage),
                [id](const auto& e) {
                    return e.id == id;
                }
            );
            if (it == std::cend(m_storage))
                return { };

            return *it;
        }

        [[nodiscard]]
        std::vector<Object>
        get(const std::size_t limit, const std::size_t offset)
        {
            std::vector<Object> ret;
            ret.reserve(limit);

            std::lock_guard lock(m_mtx);

            for (std::size_t i = 0; i < limit; i++) {
                const std::size_t index = i + offset;
                if (index >= m_storage.size())
                    break;

                ret.emplace_back(m_storage[index]);
            }

            return ret;
        }

        [[nodiscard]]
        bool
        remove(const std::size_t id)
        {
            std::lock_guard lock(m_mtx);

            return (
                std::erase_if(
                    m_storage,
                    [id](const auto& e) {
                        return e.id == id;
                    }
                )
            ) > 0;
        }

        std::optional<Object>
        modify(const std::size_t id, Object&& obj_mod)
        {
            std::lock_guard lock(m_mtx);

            auto it = std::find_if(
                std::begin(m_storage),
                std::end(m_storage),
                [id](const Object& obj) {
                    return obj.id == id;
                }
            );
            if (it == std::end(m_storage))
                return { };

            // ToDo: Merge appropriately: Need to find a way to only assign values contained in obj_mod but also being able to reset them?
            *it = std::move(obj_mod);

            // Keep ID
            it->id = id;

            return *it;
        }

    private:
        std::vector<Object> m_storage;
        std::mutex m_mtx;
        std::size_t m_id_counter = 0;

        [[nodiscard]]
        std::size_t
        generate_id()
        {
            return m_id_counter++;
        }
    };

}
