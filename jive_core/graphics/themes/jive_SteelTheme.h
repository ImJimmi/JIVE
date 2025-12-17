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
        laf.addStyles(jive::Styles{}
                          .withBorderFill(juce::Colours::transparentBlack)
                          .withBorderRadius(steel::borderRadius)
                          .withBorderWidth(steel::borderWidth)
                          .withFontFamily("Georgia")
                          .withFontPointSize(15.0f)
                          .withForeground(steel::azure));

        // Top-level components
        laf.addStyles(
            [](const auto& component) {
                return dynamic_cast<juce::TopLevelWindow*>(component.getParentComponent()) != nullptr
#if JIVE_IS_PLUGIN_PROJECT
                    || dynamic_cast<juce::AudioProcessorEditor*>(component.getParentComponent()) != nullptr
#endif
                    ;
            },
            jive::Styles{}
                .withBackground(steel::eerieBlack)
                .withBorderRadius(0)
                .withBorderWidth(juce::BorderSize<float>{}));

        // Buttons
        laf.addStyles<juce::Button>(jive::Styles{}
                                        .withBackground(steel::raisinBlack)
                                        .withBorderFill(steel::charcoalGunmetalGradient)
                                        .withShadow(steel::widgetShadow));

        laf.addStyles<juce::Button>(jive::Styles{}
                                        .withBackground(steel::raisinBlackHover),
                                    jive::InteractionState::hover);

        laf.addStyles<juce::Button>(jive::Styles{}
                                        .withBackground(steel::raisinBlackActive),
                                    jive::InteractionState::active);

        laf.addStyles<juce::ToggleButton>(jive::Styles{}
                                              .withAccent(steel::raisinBlack)
                                              .withBorderWidth(juce::BorderSize{ 2.5f }));

        laf.addStyles<juce::ToggleButton>(jive::Styles{}
                                              .withAccent(steel::raisinBlack)
                                              .withBackground(steel::pictonBlue)
                                              .withBorderFill(juce::Colours::transparentBlack)
                                              .withForeground(steel::azure),
                                          jive::InteractionState::checked);

        laf.addStyles<juce::ToggleButton>(jive::Styles{}
                                              .withBackground(steel::pictonBlueHover)
                                              .withBorderFill(juce::Colours::transparentBlack),
                                          {
                                              jive::InteractionState::checked,
                                              jive::InteractionState::hover,
                                          });

        laf.addStyles<juce::ToggleButton>(jive::Styles{}
                                              .withBackground(steel::pictonBlueActive)
                                              .withBorderFill(juce::Colours::transparentBlack),
                                          {
                                              jive::InteractionState::checked,
                                              jive::InteractionState::active,
                                          });

        laf.addStyles<juce::TextButton>(jive::Styles{}
                                            .withBackground(steel::emeraldGreen)
                                            .withBorderFill(steel::emeraldGreen)
                                            .withForeground(steel::raisinBlack),
                                        jive::InteractionState::checked);

        laf.addStyles<juce::TextButton>(jive::Styles{}
                                            .withBackground(steel::emeraldGreenHover)
                                            .withBorderFill(steel::emeraldGreenHover),
                                        {
                                            jive::InteractionState::checked,
                                            jive::InteractionState::hover,
                                        });

        laf.addStyles<juce::TextButton>(jive::Styles{}
                                            .withBackground(steel::emeraldGreenActive)
                                            .withBorderFill(steel::emeraldGreenActive),
                                        {
                                            jive::InteractionState::checked,
                                            jive::InteractionState::active,
                                        });

        // Sliders
        laf.addStyles<juce::Slider>(jive::Styles{}
                                        .withBackground(steel::raisinBlack)
                                        .withBorderFill(steel::charcoalGunmetalGradient)
                                        .withShadow(steel::widgetShadow)
                                        .withThumb(steel::pictonSphereGradient)
                                        .withTrack(steel::charcoalGunmetalGradient));

        laf.addStyles([](const juce::Component& component) {
            if (auto* slider = dynamic_cast<const juce::Slider*>(&component))
                return slider->isBar();

            return false;
        },
                      jive::Styles{}.withTrack("linear-gradient(#6FC4F5, #4FADE4, #3092CB)"));

        // Combo boxes
        laf.addStyles<juce::ComboBox>(jive::Styles{}
                                          .withBackground(steel::raisinBlack)
                                          .withBorderFill(steel::charcoalGunmetalGradient)
                                          .withShadow(steel::widgetShadow));

        laf.addStyles<juce::ComboBox>(jive::Styles{}
                                          .withBackground(steel::raisinBlackHover)
                                          .withBorderFill(steel::charcoalGunmetalGradient),
                                      jive::InteractionState::hover);

        laf.addStyles<juce::ComboBox>(jive::Styles{}
                                          .withBackground(steel::raisinBlackActive)
                                          .withBorderFill(steel::charcoalGunmetalGradient),
                                      jive::InteractionState::active);

        // Text fields
        laf.addStyles<juce::TextEditor>(jive::Styles{}
                                            .withBackground(steel::raisinBlack)
                                            .withBorderFill(steel::charcoalGunmetalGradient)
                                            .withShadow(steel::widgetShadow));

        laf.addStyles<juce::TextEditor>(jive::Styles{}
                                            .withBorderFill(steel::pictonBlue),
                                        jive::InteractionState::focus);

        laf.addStyles<jive::Caret>(jive::Styles{}
                                       .withBackground(steel::pictonBlue));

        // Scroll bars
        laf.addStyles<juce::ScrollBar>(jive::Styles{}
                                           .withForeground(steel::pictonBlue));

        laf.addStyles<juce::ScrollBar>(jive::Styles{}
                                           .withForeground(steel::pictonBlueHover),
                                       jive::InteractionState::hover);

        laf.addStyles<juce::ScrollBar>(jive::Styles{}
                                           .withForeground(steel::pictonBlueActive),
                                       jive::InteractionState::active);

        // Progress bars
        laf.addStyles<juce::ProgressBar>(jive::Styles{}
                                             .withBackground(steel::raisinBlack)
                                             .withBorderFill(steel::charcoalGunmetalGradient)
                                             .withBorderRadius(100)
                                             .withShadow(steel::widgetShadow)
                                             .withTrack("linear-gradient(270deg, #20A26E, #26C485, #3EE2A0)"));

        // Groups
        laf.addStyles<juce::GroupComponent>(jive::Styles{}
                                                .withBackground(steel::eerieBlack)
                                                .withBorderFill(steel::charcoalGunmetalGradient)
                                                .withShadow(steel::widgetShadow));
    }
} // namespace jive::themes
