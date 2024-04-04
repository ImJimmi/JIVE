#pragma once

#include <jive_core/geometry/jive_Orientation.h>

#include <juce_graphics/juce_graphics.h>

namespace jive
{
    struct Gradient
    {
        struct ColourStop
        {
            double proportion;
            juce::Colour colour;

            bool operator==(const ColourStop& other) const;
            bool operator!=(const ColourStop& other) const;
        };

        enum class Variant
        {
            linear,
            radial,
        };

        juce::ColourGradient toJuceGradient(const juce::Rectangle<float>& bounds) const;

        bool operator==(const Gradient& other) const;
        bool operator!=(const Gradient& other) const;

        juce::Array<ColourStop> stops;
        Variant variant{ Variant::linear };
        juce::Optional<Orientation> orientation;
        juce::Optional<juce::Line<float>> startEndPoints;
    };
} // namespace jive

namespace juce
{
    template <>
    struct VariantConverter<Array<jive::Gradient::ColourStop>>
    {
        static var toVar(const Array<jive::Gradient::ColourStop>& stops);
        static Array<jive::Gradient::ColourStop> fromVar(const var& object);
    };

    template <>
    struct VariantConverter<jive::Gradient::Variant>
    {
        static var toVar(const jive::Gradient::Variant& variant);
        static jive::Gradient::Variant fromVar(const var& value);
    };

    template <>
    struct VariantConverter<jive::Gradient>
    {
        static var toVar(const jive::Gradient& gradient);
        static jive::Gradient fromVar(const var& value);
    };
} // namespace juce
