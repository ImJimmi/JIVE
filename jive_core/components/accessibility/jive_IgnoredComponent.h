#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    class IgnoredComponent : public juce::Component
    {
    public:
        IgnoredComponent();

        void paint(juce::Graphics& g) override;
        std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;
    };
} // namespace jive
