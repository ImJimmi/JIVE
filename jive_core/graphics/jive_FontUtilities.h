#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    struct Styles;

    [[nodiscard]] float calculateStringWidth(const juce::String& text, const juce::Font& font);

    // Parses the given string to a `juce::Font` using CSS syntax.
    // See https://developer.mozilla.org/en-US/docs/Web/CSS/font.
    [[nodiscard]] juce::Font parseFont(const juce::String&);

    [[nodiscard]] juce::Font getFont(const juce::Component& component,
                                     const Styles& styles,
                                     juce::Font defaultFont = parseFont(""));
    [[nodiscard]] juce::Font getFont(const juce::Label& label,
                                     const Styles& styles);
    [[nodiscard]] juce::Font getFont(const juce::PopupMenu::Item& item,
                                     const juce::Component& component,
                                     const Styles& styles);
    [[nodiscard]] juce::Font getFont(const juce::ComboBox& comboBox,
                                     const Styles& styles);
} // namespace jive
