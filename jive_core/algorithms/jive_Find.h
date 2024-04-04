#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace jive
{
    juce::ValueTree find(const juce::ValueTree& root,
                         std::function<bool(const juce::ValueTree&)> predicate);
} // namespace jive
