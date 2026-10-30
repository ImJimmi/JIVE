#pragma once

#include <jive_core/jive_core.h>

namespace jive::themes
{
    namespace synthwave
    {
        static constexpr auto indigo = "#2E1F52";
        static constexpr auto indigoHover = "#3B2967";
        static constexpr auto indigoActive = "#25193F";
        static constexpr auto midnight = "#1F1238";
        static constexpr auto haze = "#FFF0FB";
        static constexpr auto neonPink = "#FF4FD8";
        static constexpr auto neonPinkHover = "#FF7AE2";
        static constexpr auto neonCyan = "#3DE8FF";
        static constexpr auto neonCyanHover = "#7AF0FF";
        static constexpr auto neonCyanActive = "#22C4DB";

        static constexpr auto duskGradient = "linear-gradient(to bottom, #4B3584, #8E4A92)";
        static constexpr auto neonGradient = "linear-gradient(to right, #FF4FD8, #3DE8FF)";
        static constexpr auto sunsetGradient = "linear-gradient(to right, #FF3CAC, #FF8A3D, #FFD34F)";
        static constexpr auto sunsetGradientHover = "linear-gradient(to right, #FF66BF, #FFA05C, #FFDE70)";
        static constexpr auto sunsetGradientActive = "linear-gradient(to right, #E62E98, #EB7A2E, #F0C23A)";
        static constexpr auto cyanSphereGradient = "radial-gradient(at 30% 30%, #E8FDFF 0%, #3DE8FF 35%, #1AA9C2 65%, #0A4458 100%)";
        static constexpr auto neonGlow = "0 0 10px rgba(255, 79, 216, 0.5)";

        static const juce::BorderSize borderWidth{ 2.0f };
        static const jive::BorderRadii borderRadius{ 4.0f };
    } // namespace synthwave

    inline void applySynthwave(LookAndFeel& laf)
    {
        // Global styles
        laf.addStyles(StyleSelector::makeSystemStyleSelector("*"),
                      jive::Styles{}
                          .withBorderFill(juce::Colours::transparentBlack)
                          .withBorderRadius(synthwave::borderRadius)
                          .withBorderWidth(synthwave::borderWidth)
                          .withFontFamily("<sans-serif>")
                          .withFontPointSize(14.0f)
                          .withForeground(synthwave::haze)
                          .withTransitions(jive::Transitions::fromString(R"(
                            background 200ms ease-in-out,
                            border 200ms ease-in-out,
                            foreground 200ms ease-in-out
                          )")));

        // Top-level components
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Editor, Window"),
                      jive::Styles{}
                          .withBackground(synthwave::duskGradient)
                          .withBorderRadius(0)
                          .withBorderWidth(juce::BorderSize<float>{}));

        // Buttons
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Button, Checkbox, Hyperlink"),
                      jive::Styles{}
                          .withBackground(synthwave::indigo)
                          .withBorderFill(synthwave::neonGradient)
                          .withShadow(synthwave::neonGlow));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Button:hover, Checkbox:hover, Hyperlink:hover"),
                      jive::Styles{}
                          .withBackground(synthwave::indigoHover));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Button:active, Checkbox:active, Hyperlink:active"),
                      jive::Styles{}
                          .withBackground(synthwave::indigoActive));

        laf.addStyles(StyleSelector::makeSystemStyleSelector("Checkbox"),
                      jive::Styles{}
                          .withAccent(synthwave::midnight));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Checkbox:checked"),
                      jive::Styles{}
                          .withBackground(synthwave::neonCyan));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Checkbox:checked:hover"),
                      jive::Styles{}
                          .withBackground(synthwave::neonCyanHover));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Checkbox:checked:active"),
                      jive::Styles{}
                          .withBackground(synthwave::neonCyanActive));

        laf.addStyles(StyleSelector::makeSystemStyleSelector("Button:checked"),
                      jive::Styles{}
                          .withBackground(synthwave::sunsetGradient)
                          .withBorderFill(synthwave::sunsetGradient)
                          .withForeground(synthwave::midnight));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Button:checked:hover"),
                      jive::Styles{}
                          .withBackground(synthwave::sunsetGradientHover)
                          .withBorderFill(synthwave::sunsetGradientHover));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Button:checked:active"),
                      jive::Styles{}
                          .withBackground(synthwave::sunsetGradientActive)
                          .withBorderFill(synthwave::sunsetGradientActive));

        // Hyperlinks
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Hyperlink"),
                      jive::Styles{}
                          .withFontStyleFlags({ juce::Font::underlined })
                          .withForeground(synthwave::neonCyan));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Hyperlink:hover"),
                      jive::Styles{}
                          .withForeground(synthwave::neonCyanHover));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Hyperlink:hover:active"),
                      jive::Styles{}
                          .withForeground(synthwave::neonCyanActive));

        // Sliders
        laf.addStyles(StyleSelector::makeSystemStyleSelector("Slider, Knob"),
                      jive::Styles{}
                          .withBackground(synthwave::indigo)
                          .withBorderFill(synthwave::neonGradient)
                          .withShadow(synthwave::neonGlow)
                          .withThumb(synthwave::cyanSphereGradient)
                          .withTrack(synthwave::sunsetGradient));

        // Combo boxes
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ComboBox"),
                      jive::Styles{}
                          .withBackground(synthwave::indigo)
                          .withBorderFill(synthwave::neonGradient)
                          .withShadow(synthwave::neonGlow));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ComboBox:hover"),
                      jive::Styles{}
                          .withBackground(synthwave::indigoHover));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ComboBox:active"),
                      jive::Styles{}
                          .withBackground(synthwave::indigoActive));

        // Text fields
        laf.addStyles(StyleSelector::makeSystemStyleSelector("TextEditor"),
                      jive::Styles{}
                          .withBackground(synthwave::indigo)
                          .withBorderFill(synthwave::neonGradient)
                          .withShadow(synthwave::neonGlow));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("TextEditor:focus"),
                      jive::Styles{}
                          .withBorderFill(synthwave::neonCyan));

        // Scroll bars
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ScrollBar"),
                      jive::Styles{}
                          .withForeground(synthwave::neonPink));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ScrollBar:hover"),
                      jive::Styles{}
                          .withForeground(synthwave::neonPinkHover));
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ScrollBar:active"),
                      jive::Styles{}
                          .withForeground(synthwave::neonCyan));

        // Progress bars
        laf.addStyles(StyleSelector::makeSystemStyleSelector("ProgressBar"),
                      jive::Styles{}
                          .withBackground(synthwave::indigo)
                          .withBorderFill(synthwave::neonGradient)
                          .withShadow(synthwave::neonGlow)
                          .withTrack(synthwave::sunsetGradient));
    }
} // namespace jive::themes
