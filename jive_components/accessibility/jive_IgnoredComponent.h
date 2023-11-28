#pragma once

namespace jive
{
    class IgnoredComponent : public juce::Component
    {
    public:
        IgnoredComponent();
        std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;
    };
} // namespace jive
