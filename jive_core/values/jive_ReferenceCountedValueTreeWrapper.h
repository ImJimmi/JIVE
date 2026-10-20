#pragma once

namespace jive
{
    struct ReferenceCountedValueTreeWrapper : public juce::ReferenceCountedObject
    {
        using Ptr = juce::ReferenceCountedObjectPtr<ReferenceCountedValueTreeWrapper>;

        explicit ReferenceCountedValueTreeWrapper(const juce::ValueTree& tree)
            : state{ tree }
        {
        }

        juce::ValueTree state;
    };
} // namespace jive
