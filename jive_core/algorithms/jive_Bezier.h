#pragma once

#include <jive_core/geometry/jive_BorderRadii.h>

#include <juce_graphics/juce_graphics.h>

namespace jive
{
    [[nodiscard]] inline auto cubicBezier(juce::Point<float> startControlPoint,
                                          juce::Point<float> endControlPoint)
    {
        juce::Path path;

        path.startNewSubPath(0.0f, 0.0f);
        path.cubicTo(startControlPoint,
                     endControlPoint,
                     juce::Point{ 1.0f, 1.0f });

        return path;
    }

    struct CubicBezier
    {
        CubicBezier() = default;
        CubicBezier(const CubicBezier&) = default;
        CubicBezier(CubicBezier&&) = default;
        CubicBezier& operator=(const CubicBezier&) = default;
        CubicBezier& operator=(CubicBezier&&) = default;
        ~CubicBezier() = default;

        explicit CubicBezier(const juce::Point<float>& initialPosition)
            : start{ initialPosition }
            , control1{ initialPosition }
            , control2{ initialPosition }
            , end{ initialPosition }
        {
        }

        [[nodiscard]] static std::array<CubicBezier, 4> makeRoundedRectangle(BorderRadii<float> radii,
                                                                             juce::Rectangle<float> bounds,
                                                                             juce::BorderSize<float> borderWidth = juce::BorderSize<float>{});

        juce::Point<float> start{ 0.0f, 0.0f };
        juce::Point<float> control1{ 0.0f, 0.0f };
        juce::Point<float> control2{ 1.0f, 1.0f };
        juce::Point<float> end{ 1.0f, 1.0f };
    };
} // namespace jive
