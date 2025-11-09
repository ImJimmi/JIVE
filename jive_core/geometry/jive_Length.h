#pragma once

#include <jive_core/algorithms/jive_Interpolate.h>

#include <juce_core/juce_core.h>

namespace jive
{
    struct Length
    {
        enum class Unit
        {
            absolute,
            percentage,
        };

        Length() = default;
        Length(const Length&) = default;
        Length& operator=(const Length&) = default;
        Length(Length&&) = default;
        Length& operator=(Length&&) = default;
        ~Length() = default;

        Length(float initialValue, Unit initialUnit)
            : value{ initialValue }
            , unit{ initialUnit }
        {
        }

        template <typename Arithmetic>
        Length(Arithmetic absoluteValue)
            : value{ absoluteValue }
            , unit{ Unit::absolute }
        {
        }

        Length(const juce::String& text)
        {
            *this = fromString(text);
        }

        Length(const char* text)
            : Length{ juce::String{ text } }
        {
        }

        [[nodiscard]] float toPixels(float percentageScale) const
        {
            return unit == Unit::percentage ? (percentageScale * value / 100.0f) : value;
        }

        [[nodiscard]] bool operator==(const Length&) const;

        [[nodiscard]] juce::String toString() const;
        [[nodiscard]] static Length fromString(const juce::String&);

        float value{ 0.0f };
        Unit unit = Unit::absolute;
    };

    template <>
    struct Interpolate<Length>
    {
        [[nodiscard]] auto operator()(const Length& start,
                                      const Length& end,
                                      double proportion) const
        {
            // You can only interpolate between lengths that share the same units.
            jassert(start.unit == end.unit);

            return Length{
                interpolate(start.value, end.value, proportion),
                start.unit,
            };
        }
    };
} // namespace jive

namespace juce
{
    template <>
    struct VariantConverter<jive::Length>
    {
        static var toVar(const jive::Length& length);
        static jive::Length fromVar(const var& value);
    };
} // namespace juce
