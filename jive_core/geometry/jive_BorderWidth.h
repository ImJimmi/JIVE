#pragma once

#include <jive_core/graphics/jive_Styles.h>

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    [[nodiscard]] juce::BorderSize<float> getBorderWidth(const juce::Component& component,
                                                         const Styles& styles,
                                                         juce::BorderSize<float> defaultThickness = juce::BorderSize<float>{});
    [[nodiscard]] juce::BorderSize<float> getBorderWidth(const juce::Label& label,
                                                         const Styles& styles);
    [[nodiscard]] juce::BorderSize<float> getBorderWidth(const juce::Button& component,
                                                         const Styles& styles);
    [[nodiscard]] juce::BorderSize<float> getBorderWidth(const juce::TextEditor& textEditor,
                                                         const Styles& styles);
    [[nodiscard]] juce::BorderSize<float> getBorderWidth(const juce::PopupMenu& popup,
                                                         const Styles& styles);
    [[nodiscard]] juce::BorderSize<float> getBorderWidth(const juce::PopupMenu::Item& item,
                                                         const Styles& styles);
    [[nodiscard]] juce::BorderSize<float> getBorderWidth(const juce::ComboBox& comboBox,
                                                         const Styles& styles);
    [[nodiscard]] juce::BorderSize<float> getBorderWidth(const juce::GroupComponent& group,
                                                         const Styles& styles);
    [[nodiscard]] juce::BorderSize<float> getBorderWidth(const juce::Slider& slider,
                                                         const Styles& styles);
    [[nodiscard]] juce::BorderSize<float> getBorderWidth(const juce::ProgressBar& bar,
                                                         const Styles& styles);
} // namespace jive
