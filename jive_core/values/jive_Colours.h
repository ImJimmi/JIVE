#pragma once

#include <juce_graphics/juce_graphics.h>

namespace jive
{
    [[nodiscard]] juce::Colour parseColour(const juce::String& colourString);
} // namespace jive
