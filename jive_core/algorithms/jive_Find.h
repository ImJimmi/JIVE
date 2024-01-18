#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace jive
{
    [[nodiscard]] juce::ValueTree find(const juce::ValueTree& root,
                                       std::function<bool(const juce::ValueTree&)> predicate);
    [[nodiscard]] juce::ValueTree findElementWithID(const juce::ValueTree& root,
                                                    const juce::Identifier& id);
} // namespace jive
