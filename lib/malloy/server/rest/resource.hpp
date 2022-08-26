#pragma once

#include "concepts.hpp"

#include <string>

namespace malloy::server::rest
{

    // ToDo: Rename to resource_endpoint
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
        resource<Object, HandlerList, HandlerImmutable, HandlerCreate, HandlerRemove, HandlerModify> r;
        r.name = std::move(name);
        r.list= std::move(list);
        r.get = std::move(get);
        r.create = std::move(create);
        r.remove = std::move(remove);
        r.modify = std::move(modify);

        return r;
    }

}
