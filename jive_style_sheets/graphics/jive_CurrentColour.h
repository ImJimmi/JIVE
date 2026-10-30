#pragma once

#include <juce_graphics/juce_graphics.h>

namespace jive
{
    // JUCE can't draw an SVG's currentColor, so it's swapped for a stand-in
    // before the SVG is parsed, and the stand-in is replaced by the actual
    // colour whenever the SVG's drawn.
    [[nodiscard]] inline juce::Colour getCurrentColourStandIn()
    {
        return juce::Colour{ 0xff010203 };
    }

    [[nodiscard]] inline juce::String replaceCurrentColourWithStandIn(const juce::String& svg)
    {
        return svg.replace("currentColor", "#" + getCurrentColourStandIn().toDisplayString(false), true);
    }
} // namespace jive
