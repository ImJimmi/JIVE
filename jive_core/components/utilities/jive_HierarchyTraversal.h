#pragma once

namespace jive
{
    template <typename ChildComponent>
    ChildComponent* find(juce::Component& parent)
    {
        for (auto* const child : parent.getChildren())
        {
            if (auto childWithType = dynamic_cast<ChildComponent*>(child))
                return childWithType;
        }

        return nullptr;
    }
} // namespace jive
