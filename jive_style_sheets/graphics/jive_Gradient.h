#pragma once

#include <jive_core/algorithms/jive_Interpolate.h>
#include <jive_core/geometry/jive_Length.h>

#include <juce_graphics/juce_graphics.h>

namespace jive
{
    enum class SideOrCorner
    {
        topLeft,
        top,
        topRight,
        right,
        bottomRight,
        bottom,
        bottomLeft,
        left,
    };

    struct RadialPosition
    {
        struct Top
        {
            Length inset;
        };
        struct Right
        {
            Length inset;
        };
        struct Bottom
        {
            Length inset;
        };
        struct Left
        {
            Length inset;
        };
        struct Centre
        {
        };

        [[nodiscard]] juce::Point<float> toPoint(const juce::Rectangle<float>& bounds) const;

        [[nodiscard]] bool operator==(const RadialPosition&) const;

        [[nodiscard]] static std::optional<RadialPosition> fromArgs(juce::StringArray&);

        std::variant<Centre, Left, Right> xAxis;
        std::variant<Centre, Top, Bottom> yAxis;
    };

    struct Gradient
    {
        enum class Variant
        {
            linear,
            radial,
        };

        struct ColourStop
        {
            [[nodiscard]] bool operator==(const ColourStop&) const;

            juce::Colour colour;
            std::optional<Length> length;
        };

        [[nodiscard]] juce::ColourGradient toJuceColourGradient(const juce::Rectangle<float>& bounds = juce::Rectangle<float>{}) const;

        [[nodiscard]] bool operator==(const Gradient&) const;

        [[nodiscard]] static std::optional<Gradient> fromString(const juce::String& gradientString);

        Variant variant;
        std::vector<ColourStop> stops;

        std::variant<float, SideOrCorner> linearAngle;

        float radialSize{ 0.0f };
        RadialPosition radialPosition;
    };

    // Gradients of the same kind with the same number of stops blend stop by
    // stop. Any others jump straight to the end.
    template <>
    struct Interpolate<Gradient>
    {
        [[nodiscard]] Gradient operator()(const Gradient& start,
                                          const Gradient& end,
                                          double proportion) const
        {
            if (start.variant != end.variant || start.stops.size() != end.stops.size())
                return end;

            auto result = end;

            for (std::size_t i = 0; i < result.stops.size(); i++)
                result.stops[i].colour = interpolate(start.stops[i].colour, end.stops[i].colour, proportion);

            if (std::holds_alternative<float>(start.linearAngle) && std::holds_alternative<float>(end.linearAngle))
                result.linearAngle = interpolate(std::get<float>(start.linearAngle), std::get<float>(end.linearAngle), proportion);

            result.radialSize = interpolate(start.radialSize, end.radialSize, proportion);
            return result;
        }
    };
} // namespace jive

namespace juce
{
    template <>
    struct VariantConverter<jive::RadialPosition>
    {
        static var toVar(const jive::RadialPosition& position);
        static jive::RadialPosition fromVar(const var& object);
    };

    template <>
    struct VariantConverter<std::variant<float, jive::SideOrCorner>>
    {
        static var toVar(const std::variant<float, jive::SideOrCorner>& linearAngle);
        static std::variant<float, jive::SideOrCorner> fromVar(const var& object);
    };

    template <>
    struct VariantConverter<std::vector<jive::Gradient::ColourStop>>
    {
        static var toVar(const std::vector<jive::Gradient::ColourStop>& stops);
        static std::vector<jive::Gradient::ColourStop> fromVar(const var& object);
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
        static String getSyntax();
    };
} // namespace juce
