#pragma once

#include <jive_core/graphics/jive_LookAndFeel.h>

namespace juce
{
    class AudioProcessorEditor
    {
    };
} // namespace juce

namespace jive::themes
{
    namespace theme
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
    } // namespace theme

    inline void applySteel(LookAndFeel& laf)
    {
        // Global styles
        laf.addStyles(jive::Styles{}
                          .withBorderFill(juce::Colours::transparentBlack)
                          .withBorderRadius(theme::borderRadius)
                          .withBorderWidth(theme::borderWidth)
                          .withFontFamily("Georgia")
                          .withFontPointSize(15.0f)
                          .withForeground(theme::azure));

        // Top-level components
        const auto topLevelStyles = jive::Styles{}
                                        .withBackground(theme::eerieBlack)
                                        .withBorderRadius(0)
                                        .withBorderWidth(juce::BorderSize<float>{});
        laf.addStyles<juce::AudioProcessorEditor>(topLevelStyles);
        laf.addStyles<juce::ResizableWindow>(topLevelStyles);

        // Buttons
        laf.addStyles<juce::Button>(jive::Styles{}
                                        .withBackground(theme::raisinBlack)
                                        .withBorderFill(theme::charcoalGunmetalGradient)
                                        .withShadow(theme::widgetShadow));

        laf.addStyles<juce::Button>(jive::Styles{}
                                        .withBackground(theme::raisinBlackHover),
                                    jive::InteractionState::hover);

        laf.addStyles<juce::Button>(jive::Styles{}
                                        .withBackground(theme::raisinBlackActive),
                                    jive::InteractionState::active);

        laf.addStyles<juce::ToggleButton>(jive::Styles{}
                                              .withAccent(theme::raisinBlack)
                                              .withBorderWidth(juce::BorderSize{ 2.5f }));

        laf.addStyles<juce::ToggleButton>(jive::Styles{}
                                              .withAccent(theme::raisinBlack)
                                              .withBackground(theme::pictonBlue)
                                              .withBorderFill(juce::Colours::transparentBlack)
                                              .withForeground(theme::azure),
                                          jive::InteractionState::toggled);

        laf.addStyles<juce::ToggleButton>(jive::Styles{}
                                              .withBackground(theme::pictonBlueHover)
                                              .withBorderFill(juce::Colours::transparentBlack),
                                          {
                                              jive::InteractionState::toggled,
                                              jive::InteractionState::hover,
                                          });

        laf.addStyles<juce::ToggleButton>(jive::Styles{}
                                              .withBackground(theme::pictonBlueActive)
                                              .withBorderFill(juce::Colours::transparentBlack),
                                          {
                                              jive::InteractionState::toggled,
                                              jive::InteractionState::active,
                                          });

        laf.addStyles<juce::TextButton>(jive::Styles{}
                                            .withBackground(theme::emeraldGreen)
                                            .withBorderFill(theme::emeraldGreen)
                                            .withForeground(theme::raisinBlack),
                                        jive::InteractionState::toggled);

        laf.addStyles<juce::TextButton>(jive::Styles{}
                                            .withBackground(theme::emeraldGreenHover)
                                            .withBorderFill(theme::emeraldGreenHover),
                                        {
                                            jive::InteractionState::toggled,
                                            jive::InteractionState::hover,
                                        });

        laf.addStyles<juce::TextButton>(jive::Styles{}
                                            .withBackground(theme::emeraldGreenActive)
                                            .withBorderFill(theme::emeraldGreenActive),
                                        {
                                            jive::InteractionState::toggled,
                                            jive::InteractionState::active,
                                        });

        // Sliders
        laf.addStyles<juce::Slider>(jive::Styles{}
                                        .withBackground(theme::raisinBlack)
                                        .withBorderFill(theme::charcoalGunmetalGradient)
                                        .withShadow(theme::widgetShadow)
                                        .withThumb(theme::pictonSphereGradient)
                                        .withTrack(theme::charcoalGunmetalGradient));

        laf.addStyles([](const juce::Component& component) {
            if (auto* slider = dynamic_cast<const juce::Slider*>(&component))
                return slider->isBar();

            return false;
        },
                      jive::Styles{}.withTrack("linear-gradient(#6FC4F5, #4FADE4, #3092CB)"));

        // Combo boxes
        laf.addStyles<juce::ComboBox>(jive::Styles{}
                                          .withBackground(theme::raisinBlack)
                                          .withBorderFill(theme::charcoalGunmetalGradient)
                                          .withShadow(theme::widgetShadow));

        laf.addStyles<juce::ComboBox>(jive::Styles{}
                                          .withBackground(theme::raisinBlackHover)
                                          .withBorderFill(theme::charcoalGunmetalGradient),
                                      jive::InteractionState::hover);

        laf.addStyles<juce::ComboBox>(jive::Styles{}
                                          .withBackground(theme::raisinBlackActive)
                                          .withBorderFill(theme::charcoalGunmetalGradient),
                                      jive::InteractionState::active);

        // Text fields
        laf.addStyles<juce::TextEditor>(jive::Styles{}
                                            .withBackground(theme::raisinBlack)
                                            .withBorderFill(theme::charcoalGunmetalGradient)
                                            .withShadow(theme::widgetShadow));

        laf.addStyles<juce::TextEditor>(jive::Styles{}
                                            .withBorderFill(theme::pictonBlue),
                                        jive::InteractionState::focus);

        laf.addStyles<jive::Caret>(jive::Styles{}
                                       .withBackground(theme::pictonBlue));

        // Scroll bars
        laf.addStyles<juce::ScrollBar>(jive::Styles{}
                                           .withForeground(theme::pictonBlue));

        laf.addStyles<juce::ScrollBar>(jive::Styles{}
                                           .withForeground(theme::pictonBlueHover),
                                       jive::InteractionState::hover);

        laf.addStyles<juce::ScrollBar>(jive::Styles{}
                                           .withForeground(theme::pictonBlueActive),
                                       jive::InteractionState::active);

        // Progress bars
        laf.addStyles<juce::ProgressBar>(jive::Styles{}
                                             .withBackground(theme::raisinBlack)
                                             .withBorderFill(theme::charcoalGunmetalGradient)
                                             .withBorderRadius(100)
                                             .withShadow(theme::widgetShadow)
                                             .withTrack("linear-gradient(270deg, #20A26E, #26C485, #3EE2A0)"));

        // Groups
        laf.addStyles<juce::GroupComponent>(jive::Styles{}
                                                .withBackground(theme::eerieBlack)
                                                .withBorderFill(theme::charcoalGunmetalGradient)
                                                .withShadow(theme::widgetShadow));
    }
} // namespace jive::themes
