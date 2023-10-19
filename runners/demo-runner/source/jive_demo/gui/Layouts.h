#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace jive_demo
{
    void replaceTextElementsWithProperties(juce::XmlElement& xml,
                                           juce::Identifier propertyName = "text")
    {
        auto text = xml.getStringAttribute(propertyName, "");

        for (auto* child : xml.getChildIterator())
        {
            if (child->isTextElement())
                text += child->getText();
            else
                replaceTextElementsWithProperties(*child, propertyName);
        }

        if (text.isNotEmpty())
            xml.setAttribute(propertyName, text);

        xml.deleteAllTextElements();
    }

    juce::ValueTree parseLayout(const void* data, int size)
    {
        auto xml = juce::parseXML(juce::String::createStringFromData(data, size));

        if (xml == nullptr)
            return {};

        replaceTextElementsWithProperties(*xml);

        return juce::ValueTree::fromXml(*xml);
    }
} // namespace jive_demo
