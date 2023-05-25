#pragma once

namespace jive
{
    juce::ValueTree find(juce::ValueTree& root,
                         std::function<bool(const juce::ValueTree&)> predicate);
} // namespace jive