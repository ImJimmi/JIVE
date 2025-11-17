#include "jive_IgnoredComponent.h"

#include <jive_core/graphics/default-styles/jive_DefaultComponentStyles.h>
#include <jive_core/jive_core.h>

namespace jive
{
    IgnoredComponent::IgnoredComponent()
    {
        setInterceptsMouseClicks(false, true);
    }

    void IgnoredComponent::paint(juce::Graphics& g)
    {
        if (auto* lookAndFeel = dynamic_cast<LookAndFeel*>(&getLookAndFeel()))
            lookAndFeel->drawComponent(g, *this);
    }

    std::unique_ptr<juce::AccessibilityHandler> IgnoredComponent::createAccessibilityHandler()
    {
        return createIgnoredAccessibilityHandler(*this);
    }
} // namespace jive
