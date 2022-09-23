#pragma once

#include <nlohmann/json.hpp>

#include <optional>

namespace malloy::rest
{

    /**
     * Literal class type that wraps a constant expression string.
     *
     * Uses implicit conversion to allow templates to *seemingly* accept constant strings.
     */
    template<size_t N>
    struct string_literal
    {
        char value[N];

        constexpr
        string_literal(const char (&str)[N])
        {
            std::copy_n(str, N, value);
        }

        [[nodiscard]]
        consteval
        std::size_t
        size() const noexcept
        {
            return sizeof(value);
        }
    };

    enum field_flags
    {
        optional = 0x01,
    };

    template<typename T, string_literal Name, field_flags Flags = static_cast<field_flags>(0x00)>
    struct field
    {
        using type = T;


        [[nodiscard]]
        static
        constexpr
        bool
        is_optional() noexcept
        {
            return Flags & field_flags::optional;
        }

        typename std::conditional_t<is_optional(), std::optional<T>, T> data;

        field() = default;

        field(const T& t) :
            data{ t }
        {
        }

        field(T&& t) :
            data{ std::move(t) }
        {
        }

        [[nodiscard]]
        constexpr
        bool
        has_value() const noexcept
        {
            if constexpr (is_optional())
                return data.has_value();
            else
                return true;
        }

        [[nodiscard]]
        constexpr
        const T&
        value() const
        {
            if constexpr (is_optional())
                return data.value();
            else
                return data;
        }

        [[nodiscard]]
        constexpr
        T
        value_or(T&& t) const
        {
            if constexpr (is_optional())
                return data.value_or(std::forward<T>(t));
            else
                return data;
        }


        [[nodiscard]]
        constexpr
        explicit
        operator T() const noexcept
        {
            if constexpr (is_optional())
                return data.value();
            else
                return data;
        }

        [[nodiscard]]
        constexpr
        explicit
        operator bool() const noexcept
        {
            return this->has_value();
        }

        constexpr
        auto
        operator=(const T& t) noexcept
        requires std::is_copy_assignable_v<T>
        {
            data = t;

            return *this;
        }

        constexpr
        auto
        operator=(T&& t) noexcept
        requires std::is_move_assignable_v<T>
        {
            data = std::move(t);

            return *this;
        }

        constexpr
        bool
        operator==(const T& rhs) const
        {
            if constexpr (is_optional())
                if constexpr (has_value())
                    return *data == rhs;
                else
                    return false;
            else
                return data == rhs;
        }

        [[nodiscard]]
        static
        constexpr
        const char*
        name() noexcept
        {
            return Name.value;
        }
    };

    template<typename Field>
    static
    //requires std::derived_from<field>     // ToDo
    void
    field_to_json(nlohmann::json& j, const Field& field)
    {
        if /* constexpr */ (field.has_value())      // ToDo
            j[field.name()] = field.value();
    }

    template<typename Field>
    static
    // requires // ToDo
    void
    field_from_json(const nlohmann::json& j, Field& field)      // ToDo: Move from json?
    {
        if (!j.contains<const char*>(field.name()))
            return;

        if (j.is_null()) {
            if constexpr (Field::is_optional())
                field.data.reset();
        }
        else
            field.data = j[field.name()];
    }

}
