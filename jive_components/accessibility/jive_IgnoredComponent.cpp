#include "jive_IgnoredComponent.h"

namespace jive
{
    IgnoredComponent::IgnoredComponent()
    {
        setInterceptsMouseClicks(false, true);
    }

    std::unique_ptr<juce::AccessibilityHandler> IgnoredComponent::createAccessibilityHandler()
    {
        return createIgnoredAccessibilityHandler(*this);
    }
} // namespace jive
