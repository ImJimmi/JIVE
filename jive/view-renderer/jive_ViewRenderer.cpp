#include "jive_ViewRenderer.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    std::unique_ptr<juce::Component> ViewRenderer::createView(juce::ValueTree sourceTree) const
    {
        juce::ignoreUnused(sourceTree);
        return nullptr;
    }
} // namespace jive
