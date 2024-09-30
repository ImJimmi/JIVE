#pragma once

#include <juce_graphics/juce_graphics.h>

namespace jive
{
    [[nodiscard]] int parseFontStyleFlags(const juce::String& styleString);
    [[nodiscard]] float calculateStringWidth(const juce::String& text, const juce::Font& font);
} // namespace jive
