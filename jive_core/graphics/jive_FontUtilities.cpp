#include "jive_FontUtilities.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
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
} // namespace jive
