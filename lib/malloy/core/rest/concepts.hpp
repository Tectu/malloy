#pragma once

#include <nlohmann/json_fwd.hpp>

#include <concepts>

// ToDo: Maybe add ::concepts namespace?
namespace malloy::rest
{
    struct response_handle;

    /**
     * Ensures that T has an inner type member 'id_type'
     */
    template<typename T>
    concept has_id_type = requires(T) {
        typename T::id_type;
    };

    template<typename T>
    concept has_to_json = requires(T t) {
        { t.to_json() } -> std::same_as<nlohmann::json>;
    };

    template<typename T>
    concept has_from_json = requires(T t, nlohmann::json&& j) {
        { t.from_json(std::move(j)) } -> std::same_as<bool>;
    };

    // ToDo: Rename to resource
    // ToDo: Ensure that we can convert a T::id_type to an std::string somehow
    // ToDo: For JSON stuff maybe use std::is_convertible instead of exact type?
    template<typename T>
    concept object =
        has_id_type<T> &&
        has_to_json<T> &&
        has_from_json<T>;

    /**
     * Handler to list existing resources (with pagination support).
     */
    template<typename Func>
    concept handler_list =
        std::invocable<Func, std::size_t, std::size_t>
        && requires(Func f, std::size_t limit, std::size_t offset) {
            { std::invoke(f, limit, offset) } -> std::same_as<response_handle>;
    };

    /**
     * Handler which only identifies a resource.
     */
    template<typename Func, typename Object>
    concept handler_identity =
        std::invocable<Func, typename Object::id_type>
        && requires(Object) {
            object<Object>;
        }
        && requires(Func f, const typename Object::id_type& id) {
            { std::invoke(f, id) } -> std::same_as<response_handle>;
    };

    /**
     * Handler to retrieve an existing resource.
     */
    template<typename Func, typename Object>
    concept handler_immutable =
        std::invocable<Func, typename Object::id_type>
        && requires(Object) {
            object<Object>;
        }
        && requires(Func f, const typename Object::id_type& id) {
            { std::invoke(f, id) } -> std::same_as<response_handle>;
    };

    /**
     * Handler to modify an existing resource.
     */
    template<typename Func, typename Object>
    concept handler_mutable =
        std::invocable<Func, typename Object::id_type, Object>
        && requires(Object) {
            object<Object>;
        }
        && requires(Func f, const typename Object::id_type& id, Object&& obj) {
            { std::invoke(f, id, std::forward<Object>(obj)) } -> std::same_as<response_handle>;
    };

    template<typename Func>
    concept handler_create =
        std::invocable<Func>
        && requires(Func f) {
            { std::invoke(f) } -> std::same_as<response_handle>;
    };

}
