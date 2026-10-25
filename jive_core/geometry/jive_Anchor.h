#pragma once

#include "jive_Length.h"

namespace jive
{
    struct Anchor
    {
        [[nodiscard]] bool operator==(const Anchor&) const;

        [[nodiscard]] juce::String toString() const;
        [[nodiscard]] static Anchor fromString(const juce::String&);

        Length x{ 0.0f, Length::Unit::percentage };
        Length y{ 0.0f, Length::Unit::percentage };
    };

    template <>
    struct Interpolate<Anchor>
    {
        [[nodiscard]] auto operator()(const Anchor& start,
                                      const Anchor& end,
                                      double proportion) const
        {
            return Anchor{
                interpolate(start.x, end.x, proportion),
                interpolate(start.y, end.y, proportion),
            };
        }
    };
} // namespace jive

namespace juce
{
    template <>
    struct VariantConverter<jive::Anchor>
    {
        static var toVar(const jive::Anchor& anchor);
        static jive::Anchor fromVar(const var& value);
    };
} // namespace juce
