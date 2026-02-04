#pragma once

#include <jive_core/jive_core.h>

namespace jive::themes
{
    namespace steel
    {
        static constexpr auto eerieBlack = "#1E2125";
        static constexpr auto raisinBlack = "#22262D";
        static constexpr auto raisinBlackHover = "#262D36";
        static constexpr auto raisinBlackActive = "#1E2227";
        static constexpr auto charcoal = "#3F4B5D";
        static constexpr auto gunmetal = "#2C3542";
        static constexpr auto emeraldGreen = "#26C485";
        static constexpr auto emeraldGreenHover = "#3EE2A0";
        static constexpr auto emeraldGreenActive = "#20A26E";
        static constexpr auto pictonBlue = "#4FADE4";
        static constexpr auto pictonBlueHover = "#6FC4F5";
        static constexpr auto pictonBlueActive = "#3092CB";
        static constexpr auto azure = "#E6F9FF";

        static constexpr auto charcoalGunmetalGradient = "linear-gradient(-7.5deg, #3F4B5D, #2C3542)";
        static constexpr auto pictonSphereGradient = "radial-gradient(at 30% 30%, #AED5EC 0%, #4FADE4 30%, #4392C0 60%, #214960 100%)";
        static constexpr auto widgetShadow = "2px 2px 6px rgba(0, 0, 0, 0.25)";

        static const juce::BorderSize borderWidth{ 2.0f };
        static const jive::BorderRadii borderRadius{ 6.5f };
    } // namespace steel

    inline void applySteel(LookAndFeel& laf)
    {
        // Global styles
        laf.addStyles(StyleSelector::makeSystemStyleSelector("*"),
                      jive::Styles{}
                          .withBorderFill(juce::Colours::transparentBlack)
                          .withBorderRadius(steel::borderRadius)
                          .withBorderWidth(steel::borderWidth)
                          .withFill(steel::azure)
                          .withFontFamily("<sans-serif>")
                          .withFontPointSize(14.0f)
                          .withForeground(steel::azure)
                          .withTransitions(jive::Transitions::fromString(R"(
                            background 200ms ease-in-out,
                            border 200ms ease-in-out,
                            foreground 200ms ease-in-out
                          )")));

        // Top-level components
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Editor, Window"),
                      jive::Styles{}
                          .withBackground(steel::eerieBlack)
                          .withBorderRadius(0)
                          .withBorderWidth(juce::BorderSize<float>{}));

        // Buttons
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Button"),
                      jive::Styles{}
                          .withBackground(steel::raisinBlack)
                          .withBorderFill(steel::charcoalGunmetalGradient)
                          .withShadow(steel::widgetShadow));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Button:hover"),
                      jive::Styles{}
                          .withBackground(steel::raisinBlackHover));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Button:active"),
                      jive::Styles{}
                          .withBackground(steel::raisinBlackActive));

        laf.addStyles(StyleSelector::makeSystemStyleSelector("Checkbox"),
                      jive::Styles{}
                          .withAccent(steel::raisinBlack)
                          .withBorderWidth(juce::BorderSize{ 2.5f }));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Checkbox:checked"),
                      jive::Styles{}
                          .withAccent(steel::raisinBlack)
                          .withBackground(steel::pictonBlue)
                          .withBorderFill(juce::Colours::transparentBlack)
                          .withForeground(steel::azure));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Checkbox:checked:hover"),
                      jive::Styles{}
                          .withBackground(steel::pictonBlueHover)
                          .withBorderFill(juce::Colours::transparentBlack));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Checkbox:checked:active"),
                      jive::Styles{}
                          .withBackground(steel::pictonBlueActive)
                          .withBorderFill(juce::Colours::transparentBlack));

        laf.addStyles(StyleSelector::makeSystemStyleSelector("TextButton:checked"),
                      jive::Styles{}
                          .withBackground(steel::emeraldGreen)
                          .withBorderFill(steel::emeraldGreen)
                          .withForeground(steel::raisinBlack));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("TextButton:checked:hover"),
                      jive::Styles{}
                          .withBackground(steel::emeraldGreenHover)
                          .withBorderFill(steel::emeraldGreenHover));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("TextButton:checked:active"),
                      jive::Styles{}
                          .withBackground(steel::emeraldGreenActive)
                          .withBorderFill(steel::emeraldGreenActive));

        // Sliders
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Slider"),
                      jive::Styles{}
                          .withBackground(steel::raisinBlack)
                          .withBorderFill(steel::charcoalGunmetalGradient)
                          .withShadow(steel::widgetShadow)
                          .withThumb(steel::pictonSphereGradient)
                          .withTrack(steel::charcoalGunmetalGradient));

        // Combo boxes
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ComboBox"),
                      jive::Styles{}
                          .withBackground(steel::raisinBlack)
                          .withBorderFill(steel::charcoalGunmetalGradient)
                          .withShadow(steel::widgetShadow));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ComboBox:hover"),
                      jive::Styles{}
                          .withBackground(steel::raisinBlackHover)
                          .withBorderFill(steel::charcoalGunmetalGradient));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ComboBox:active"),
                      jive::Styles{}
                          .withBackground(steel::raisinBlackActive)
                          .withBorderFill(steel::charcoalGunmetalGradient));

        // Text fields
        laf.addStyles(StyleSelector::makeSystemStyleSelector("TextEditor"),
                      jive::Styles{}
                          .withBackground(steel::raisinBlack)
                          .withBorderFill(steel::charcoalGunmetalGradient)
                          .withShadow(steel::widgetShadow));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("TextEditor:focus"),
                      jive::Styles{}
                          .withBorderFill(steel::pictonBlue));

        // Scroll bars
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ScrollBar"),
                      jive::Styles{}
                          .withForeground(steel::pictonBlue));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ScrollBar:hover"),
                      jive::Styles{}
                          .withForeground(steel::pictonBlueHover));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ScrollBar:active"),
                      jive::Styles{}
                          .withForeground(steel::pictonBlueActive));

        // Progress bars
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ProgressBar"),
                      jive::Styles{}
                          .withBackground(steel::raisinBlack)
                          .withBorderFill(steel::charcoalGunmetalGradient)
                          .withBorderRadius(100)
                          .withShadow(steel::widgetShadow)
                          .withTrack("linear-gradient(270deg, #20A26E, #26C485, #3EE2A0)"));
    }
} // namespace jive::themes
