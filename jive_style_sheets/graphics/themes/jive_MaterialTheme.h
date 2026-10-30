#pragma once

#include <jive_style_sheets/jive_style_sheets.h>

namespace jive::themes
{
    // Based on the light baseline scheme of Google's Material Design 3: flat
    // tonal fills with no borders or shadows. Hovering and pressing darken a
    // fill the way Material's state layers do, by mixing in 8% and 12% of the
    // colour drawn on top of it.
    namespace material
    {
        static constexpr auto surface = "#FEF7FF";
        static constexpr auto onSurface = "#1D1B20";
        static constexpr auto onSurfaceVariant = "#49454F";
        static constexpr auto outline = "#79747E";
        static constexpr auto surfaceContainerHighest = "#E6E0E9";
        static constexpr auto surfaceContainerHighestHover = "#D6D0D9";
        static constexpr auto surfaceContainerHighestPressed = "#CEC8D1";
        static constexpr auto primary = "#6750A4";
        static constexpr auto primaryHover = "#735EAB";
        static constexpr auto primaryPressed = "#7965AF";
        static constexpr auto onPrimary = "#FFFFFF";
        static constexpr auto secondaryContainer = "#E8DEF8";
        static constexpr auto secondaryContainerHover = "#D8CEE8";
        static constexpr auto secondaryContainerPressed = "#D0C6DF";
        static constexpr auto onSecondaryContainer = "#1D192B";

        static const jive::BorderRadii borderRadius{ 12.0f };
        static const jive::BorderRadii fieldBorderRadius{ 4.0f };
        static const jive::BorderRadii checkboxBorderRadius{ 2.0f };
        static const jive::BorderRadii fullBorderRadius{ 100.0f };
    } // namespace material

    inline void applyMaterial(LookAndFeel& laf)
    {
        // Global styles
        laf.addStyles(StyleSelector::makeSystemStyleSelector("*"),
                      jive::Styles{}
                          .withBorderFill(juce::Colours::transparentBlack)
                          .withBorderRadius(material::borderRadius)
                          .withBorderWidth(juce::BorderSize<float>{})
                          .withFontFamily("<sans-serif>")
                          .withFontPointSize(14.0f)
                          .withForeground(material::onSurface)
                          .withTransitions(jive::Transitions::fromString(R"(
                            background 200ms ease-in-out,
                            foreground 200ms ease-in-out
                          )")));

        // Top-level components
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Editor, Window"),
                      jive::Styles{}
                          .withBackground(material::surface)
                          .withBorderRadius(0));

        // Buttons, styled as Material's tonal buttons, or filled ones when
        // they're toggled on.
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Button"),
                      jive::Styles{}
                          .withBackground(material::secondaryContainer)
                          .withBorderRadius(material::fullBorderRadius)
                          .withForeground(material::onSecondaryContainer));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Button:hover"),
                      jive::Styles{}
                          .withBackground(material::secondaryContainerHover));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Button:hover:active"),
                      jive::Styles{}
                          .withBackground(material::secondaryContainerPressed));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Button:checked"),
                      jive::Styles{}
                          .withBackground(material::primary)
                          .withForeground(material::onPrimary));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Button:checked:hover"),
                      jive::Styles{}
                          .withBackground(material::primaryHover));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Button:checked:hover:active"),
                      jive::Styles{}
                          .withBackground(material::primaryPressed));

        laf.addStyles(StyleSelector::makeSystemStyleSelector("Checkbox"),
                      jive::Styles{}
                          .withAccent(material::onPrimary)
                          .withBackground(material::surfaceContainerHighest)
                          .withBorderRadius(material::checkboxBorderRadius));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Checkbox:hover"),
                      jive::Styles{}
                          .withBackground(material::surfaceContainerHighestHover));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Checkbox:hover:active"),
                      jive::Styles{}
                          .withBackground(material::surfaceContainerHighestPressed));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Checkbox:checked"),
                      jive::Styles{}
                          .withBackground(material::primary));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Checkbox:checked:hover"),
                      jive::Styles{}
                          .withBackground(material::primaryHover));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Checkbox:checked:hover:active"),
                      jive::Styles{}
                          .withBackground(material::primaryPressed));

        // Hyperlinks
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Hyperlink"),
                      jive::Styles{}
                          .withFontStyleFlags({ juce::Font::underlined })
                          .withForeground(material::primary));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Hyperlink:hover"),
                      jive::Styles{}
                          .withForeground(material::primaryHover));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Hyperlink:hover:active"),
                      jive::Styles{}
                          .withForeground(material::primaryPressed));

        // Sliders
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Slider, Knob"),
                      jive::Styles{}
                          .withBackground(material::secondaryContainer)
                          .withThumb(material::primary)
                          .withTrack(material::primary));

        // Combo boxes and text fields, styled as Material's filled fields.
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ComboBox"),
                      jive::Styles{}
                          .withBackground(material::surfaceContainerHighest)
                          .withBorderRadius(material::fieldBorderRadius));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ComboBox:hover"),
                      jive::Styles{}
                          .withBackground(material::surfaceContainerHighestHover));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ComboBox:hover:active"),
                      jive::Styles{}
                          .withBackground(material::surfaceContainerHighestPressed));

        laf.addStyles(StyleSelector::makeSystemStyleSelector("TextEditor"),
                      jive::Styles{}
                          .withBackground(material::surfaceContainerHighest)
                          .withBorderRadius(material::fieldBorderRadius));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("TextEditor:focus"),
                      jive::Styles{}
                          .withBackground(material::secondaryContainer));

        // Scroll bars
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ScrollBar"),
                      jive::Styles{}
                          .withForeground(material::outline));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ScrollBar:hover"),
                      jive::Styles{}
                          .withForeground(material::onSurfaceVariant));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ScrollBar:active"),
                      jive::Styles{}
                          .withForeground(material::onSurface));

        // Progress bars
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ProgressBar"),
                      jive::Styles{}
                          .withBackground(material::secondaryContainer)
                          .withBorderRadius(material::fullBorderRadius)
                          .withTrack(material::primary));
    }
} // namespace jive::themes
