#pragma once

#include <jive_core/geometry/jive_BorderRadii.h>

namespace jive
{
    struct Styles;

    [[nodiscard]] juce::Path getShape(BorderRadii<float> radii,
                                      juce::Rectangle<float> bounds,
                                      juce::BorderSize<float> borderWidth = juce::BorderSize<float>{});

    [[nodiscard]] juce::Rectangle<float> getTitleBounds(const juce::GroupComponent& group,
                                                        const Styles& styles);
    [[nodiscard]] juce::Rectangle<float> getGroupBounds(const juce::GroupComponent& group,
                                                        const Styles& styles);

    [[nodiscard]] juce::Rectangle<float> getTrackBounds(const juce::ProgressBar& bar,
                                                        const Styles& styles,
                                                        double progressStart,
                                                        double progressEnd);
} // namespace jive
