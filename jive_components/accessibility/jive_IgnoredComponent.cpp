#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    std::unique_ptr<juce::AccessibilityHandler> IgnoredComponent::createAccessibilityHandler()
    {
        return createIgnoredAccessibilityHandler(*this);
    }
} // namespace jive
