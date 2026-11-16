#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace jive
{
    [[nodiscard]] juce::ValueTree find(const juce::ValueTree& root,
                                       std::function<bool(const juce::ValueTree&)> predicate);
    [[nodiscard]] std::vector<juce::ValueTree> findAll(const juce::ValueTree& root,
                                                       std::function<bool(const juce::ValueTree&)> predicate);
    [[nodiscard]] juce::ValueTree findElementWithID(const juce::ValueTree& root,
                                                    const juce::Identifier& id);
    [[nodiscard]] std::vector<juce::ValueTree> findElementsWithClass(const juce::ValueTree& root,
                                                                     const juce::String& className);
    [[nodiscard]] std::vector<juce::ValueTree> findElementsWithText(const juce::ValueTree& root,
                                                                    const juce::String& text);
    [[nodiscard]] std::vector<juce::ValueTree> findElementsWithText(const std::vector<juce::ValueTree>& roots,
                                                                    const juce::String& text);
} // namespace jive
