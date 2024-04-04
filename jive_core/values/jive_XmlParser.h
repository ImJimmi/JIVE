#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace jive
{
    [[nodiscard]] juce::ValueTree parseXML(const juce::XmlElement& xml);
    [[nodiscard]] juce::ValueTree parseXML(const juce::String& xmlString);
    [[nodiscard]] juce::ValueTree parseXML(const void* xmlStringData, int xmlStringDataSize);
} // namespace jive
