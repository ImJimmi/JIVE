#include "jive_FontUtilities.h"

#include <jive_core/compile-time-helpers/jive_JuceVersion.h>

namespace jive
{
    int parseFontStyleFlags(const juce::String& styleString)
    {
        int flags = juce::Font::plain;
        const auto tokens = juce::StringArray::fromTokens(styleString, false);

        if (tokens.contains("bold"))
            flags += juce::Font::bold;
        if (tokens.contains("italic"))
            flags += juce::Font::italic;
        if (tokens.contains("underlined"))
            flags += juce::Font::underlined;

        return flags;
    }

    float calculateStringWidth(const juce::String& text, const juce::Font& font)
    {
#if JUCE_VERSION > JIVE_JUCE_VERSION(8, 0, 1)
        return juce::GlyphArrangement::getStringWidth(font, text);
#else
        return font.getStringWidthFloat(text);
#endif
    }
} // namespace jive
