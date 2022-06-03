#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class IgnoredComponent : public juce::Component
    {
        std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;
    };
} // namespace jive
