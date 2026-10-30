#pragma once

#include <jive_core/geometry/jive_BorderRadii.h>

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    struct Styles;

    [[nodiscard]] BorderRadii<float> getBorderRadius(const juce::Component& component,
                                                     const Styles& styles,
                                                     BorderRadii<float> defaultRadius = BorderRadii{ 0.0f });
    [[nodiscard]] BorderRadii<float> getBorderRadius(const juce::Button& component,
                                                     const Styles& styles);
    [[nodiscard]] BorderRadii<float> getBorderRadius(const juce::PopupMenu& popup,
                                                     const Styles& styles);
    [[nodiscard]] BorderRadii<float> getBorderRadius(const juce::PopupMenu::Item& item,
                                                     const juce::PopupMenu& popup,
                                                     const Styles& styles);
    [[nodiscard]] BorderRadii<float> getBorderRadius(const juce::ComboBox& comboBox,
                                                     const Styles& styles);
    [[nodiscard]] BorderRadii<float> getBorderRadius(const juce::GroupComponent& group,
                                                     const Styles& styles);
    [[nodiscard]] BorderRadii<float> getBorderRadius(const juce::Slider& slider,
                                                     const Styles& styles);
    [[nodiscard]] BorderRadii<float> getBorderRadius(const juce::ProgressBar& bar,
                                                     const Styles& styles);
} // namespace jive
