#pragma once

#include <juce_graphics/juce_graphics.h>

namespace jive
{
    [[nodiscard]] static auto cubicBezier(juce::Point<float> startControlPoint,
                                          juce::Point<float> endControlPoint)
    {
        juce::Path path;

        path.startNewSubPath(0.0f, 0.0f);
        path.cubicTo(startControlPoint,
                     endControlPoint,
                     juce::Point{ 1.0f, 1.0f });

        return path;
    }
} // namespace jive
