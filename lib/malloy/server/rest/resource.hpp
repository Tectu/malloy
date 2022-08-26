#pragma once

#include "../../core/rest/concepts.hpp"

#include <string>

namespace malloy::server::rest
{
    using namespace malloy::rest;

    template<
        object Object,
        handler_list HandlerList,
        handler_immutable<Object> HandlerImmutable,
        handler_mutable<Object> HandlerCreate,
        handler_mutable<Object> HandlerRemove,
        handler_mutable<Object> HandlerModify
    >
    struct resource
    {
        std::string name;

        HandlerList      list;          // GET{limit, offset}
        HandlerImmutable get;           // GET{id}
        HandlerCreate    create;        // POST{id}
        HandlerRemove    remove;        // DELETE{id}
        HandlerModify    modify;        // PATCH{id}
    };

    template<
        object Object,
        handler_list HandlerList,
        handler_immutable<Object> HandlerImmutable,
        handler_mutable<Object> HandlerCreate,
        handler_mutable<Object> HandlerRemove,
        handler_mutable<Object> HandlerModify
    >
    resource<
        Object,
        HandlerList,
        HandlerImmutable,
        HandlerCreate,
        HandlerRemove,
        HandlerModify
    >
    make_resource(
        std::string name,
        HandlerList&& list,
        HandlerImmutable&& get,
        HandlerCreate&& create,
        HandlerRemove&& remove,
        HandlerModify&& modify
    )
    {
        resource<Object, HandlerList, HandlerImmutable, HandlerCreate, HandlerRemove, HandlerModify> r{
            std::move(name),
            std::move(list),
            std::move(get),
            std::move(create),
            std::move(remove),
            std::move(modify)
        };

        return r;
    }

}
