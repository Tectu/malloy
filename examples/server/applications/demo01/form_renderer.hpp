#pragma once

#include <malloy/core/html/form_renderer.hpp>

struct form_renderer :
    malloy::html::form_renderer
{
    ~form_renderer() override = default;

    [[nodiscard]]
    std::string
    render(const malloy::html::form& f) const override;
};
