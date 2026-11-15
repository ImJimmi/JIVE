#pragma once

#include <jive_style_sheets/jive_style_sheets.h>

namespace jive::themes
{
    // JUCE's own LookAndFeel_V4 dark scheme. Most of it comes from the default
    // painters; these are the parts that JUCE draws differently.
    inline void applyJuce(LookAndFeel& laf)
    {
        const auto scrollBarThumb = juce::LookAndFeel_V4::getDarkColourScheme()
                                        .getUIColour(juce::LookAndFeel_V4::ColourScheme::UIColour::defaultFill);
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ScrollBar:hover"),
                      jive::Styles{}
                          .withThumb(scrollBarThumb.brighter(0.25f)));

        const auto hyperlink = juce::LookAndFeel_V4::getDarkColourScheme()
                                   .getUIColour(juce::LookAndFeel_V4::ColourScheme::UIColour::defaultText)
                                   .interpolatedWith(juce::Colours::blue, 0.4f);

        laf.addStyles(StyleSelector::makeSystemStyleSelector("Hyperlink"),
                      jive::Styles{}
                          .withFontStyleFlags({ juce::Font::underlined })
                          .withForeground(hyperlink));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Hyperlink:hover"),
                      jive::Styles{}
                          .withForeground(hyperlink.darker(0.4f)));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Hyperlink:hover:active"),
                      jive::Styles{}
                          .withForeground(hyperlink.darker(1.3f)));
    }
} // namespace jive::themes
