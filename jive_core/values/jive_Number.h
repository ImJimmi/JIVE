#pragma once

#include <jive_core/algorithms/jive_Interpolate.h>
#include <jive_core/values/variant-converters/jive_VariantValidation.h>

namespace jive
{
    template <typename Arithmetic, typename Tag>
    struct Number
    {
        using ValueType = Arithmetic;

        Number() = default;

        template <typename Other, typename = std::enable_if_t<std::is_arithmetic_v<Other>>>
        Number(Other initialValue)
            : value{ static_cast<Arithmetic>(initialValue) }
        {
        }

        operator Arithmetic() const
        {
            return value;
        }

        Arithmetic value{};
    };

    template <typename Arithmetic, typename Tag>
    struct Interpolate<Number<Arithmetic, Tag>>
    {
        [[nodiscard]] Number<Arithmetic, Tag> operator()(Number<Arithmetic, Tag> start,
                                                         Number<Arithmetic, Tag> end,
                                                         double proportion) const
        {
            return interpolate(start.value, end.value, proportion);
        }
    };
} // namespace jive

namespace juce
{
    template <typename Arithmetic, typename Tag>
    struct VariantConverter<jive::Number<Arithmetic, Tag>>
    {
        static jive::Number<Arithmetic, Tag> fromVar(const var& v)
        {
            return static_cast<Arithmetic>(v);
        }

        static var toVar(jive::Number<Arithmetic, Tag> number)
        {
            return number.value;
        }

        static bool isValid(const var& v)
        {
            return v.isVoid() || v.isBool() || jive::isNumber(v);
        }

        static String getSyntax()
        {
            return "a number";
        }
    };
} // namespace juce
