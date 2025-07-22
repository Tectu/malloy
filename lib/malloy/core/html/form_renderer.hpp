#pragma once

#include <string>

namespace malloy::html
{
    struct form_field;
    class form;

    /**
     * An interface for a form renderer.
     */
    struct form_renderer
    {
        virtual
        ~form_renderer() = default;

        [[nodiscard]]
        virtual
        std::string
        render(const form& f) const = 0;
    };

    /**
     * A simple form renderer.
     */
    struct form_renderer_basic :
        form_renderer
    {
        ~form_renderer_basic() override = default;

        [[nodiscard]]
        std::string
        render(const form& f) const override;
    };

}
