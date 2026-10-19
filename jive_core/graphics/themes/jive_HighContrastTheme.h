#pragma once

#include <jive_core/jive_core.h>

namespace jive::themes
{
    // Pure black and white with a single bright accent, square corners and no
    // shadows, so every edge and state reads clearly.
    namespace high_contrast
    {
        static constexpr auto black = "#000000";
        static constexpr auto white = "#FFFFFF";
        static constexpr auto grey = "#5C5C5C";
        static constexpr auto darkGrey = "#2E2E2E";
        static constexpr auto yellow = "#FFE600";
        static constexpr auto yellowHover = "#FFF066";
        static constexpr auto yellowActive = "#D9C400";

        static const juce::BorderSize borderWidth{ 2.0f };
        static const jive::BorderRadii borderRadius{ 0.0f };
    } // namespace high_contrast

    inline void applyHighContrast(LookAndFeel& laf)
    {
        // Global styles
        laf.addStyles(StyleSelector::makeSystemStyleSelector("*"),
                      jive::Styles{}
                          .withBorderFill(juce::Colours::transparentBlack)
                          .withBorderRadius(high_contrast::borderRadius)
                          .withBorderWidth(high_contrast::borderWidth)
                          .withFontFamily("<sans-serif>")
                          .withFontPointSize(14.0f)
                          .withForeground(high_contrast::white)
                          .withTransitions(jive::Transitions::fromString(R"(
                            background 100ms linear,
                            border 100ms linear,
                            foreground 100ms linear
                          )")));

        // Top-level components
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Editor, Window"),
                      jive::Styles{}
                          .withBackground(high_contrast::black)
                          .withBorderRadius(0)
                          .withBorderWidth(juce::BorderSize<float>{}));

        // Buttons
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Button"),
                      jive::Styles{}
                          .withBackground(high_contrast::black)
                          .withBorderFill(high_contrast::white));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Button:hover"),
                      jive::Styles{}
                          .withBorderFill(high_contrast::yellow));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Button:active"),
                      jive::Styles{}
                          .withBackground(high_contrast::darkGrey));

        laf.addStyles(StyleSelector::makeSystemStyleSelector("Checkbox"),
                      jive::Styles{}
                          .withAccent(high_contrast::black));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Checkbox:checked"),
                      jive::Styles{}
                          .withAccent(high_contrast::black)
                          .withBackground(high_contrast::yellow)
                          .withBorderFill(high_contrast::yellow));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Checkbox:checked:hover"),
                      jive::Styles{}
                          .withBackground(high_contrast::yellowHover)
                          .withBorderFill(high_contrast::yellowHover));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Checkbox:checked:active"),
                      jive::Styles{}
                          .withBackground(high_contrast::yellowActive)
                          .withBorderFill(high_contrast::yellowActive));

        laf.addStyles(StyleSelector::makeSystemStyleSelector("TextButton:checked"),
                      jive::Styles{}
                          .withBackground(high_contrast::yellow)
                          .withBorderFill(high_contrast::yellow)
                          .withForeground(high_contrast::black));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("TextButton:checked:hover"),
                      jive::Styles{}
                          .withBackground(high_contrast::yellowHover)
                          .withBorderFill(high_contrast::yellowHover));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("TextButton:checked:active"),
                      jive::Styles{}
                          .withBackground(high_contrast::yellowActive)
                          .withBorderFill(high_contrast::yellowActive));

        // Hyperlinks
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Hyperlink"),
                      jive::Styles{}
                          .withFontStyleFlags({ juce::Font::underlined })
                          .withForeground(high_contrast::yellow));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Hyperlink:hover"),
                      jive::Styles{}
                          .withForeground(high_contrast::yellowHover));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Hyperlink:hover:active"),
                      jive::Styles{}
                          .withForeground(high_contrast::yellowActive));

        // Sliders
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Slider, Knob"),
                      jive::Styles{}
                          .withBackground(high_contrast::grey)
                          .withBorderFill(high_contrast::white)
                          .withThumb(high_contrast::yellow)
                          .withTrack(high_contrast::white));

        // Combo boxes
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ComboBox"),
                      jive::Styles{}
                          .withBackground(high_contrast::black)
                          .withBorderFill(high_contrast::white));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ComboBox:hover"),
                      jive::Styles{}
                          .withBorderFill(high_contrast::yellow));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ComboBox:active"),
                      jive::Styles{}
                          .withBackground(high_contrast::darkGrey)
                          .withBorderFill(high_contrast::yellow));

        // Text fields
        laf.addStyles(StyleSelector::makeSystemStyleSelector("TextEditor"),
                      jive::Styles{}
                          .withBackground(high_contrast::black)
                          .withBorderFill(high_contrast::white));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("TextEditor:focus"),
                      jive::Styles{}
                          .withBorderFill(high_contrast::yellow));

        // Scroll bars
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ScrollBar"),
                      jive::Styles{}
                          .withForeground(high_contrast::white));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ScrollBar:hover"),
                      jive::Styles{}
                          .withForeground(high_contrast::yellow));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ScrollBar:active"),
                      jive::Styles{}
                          .withForeground(high_contrast::yellowActive));

        // Progress bars
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ProgressBar"),
                      jive::Styles{}
                          .withBackground(high_contrast::black)
                          .withBorderFill(high_contrast::white)
                          .withTrack(high_contrast::yellow));
    }
} // namespace jive::themes
