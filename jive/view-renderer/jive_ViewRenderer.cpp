#include "jive_ViewRenderer.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    bool treeHasMatchingTypeIgnoringCase (const juce::ValueTree& tree, const juce::String& expectedType)
    {
        return tree.getType().toString().equalsIgnoreCase(expectedType);
    }

    //==================================================================================================================
    std::unique_ptr<juce::Component> ViewRenderer::createView(juce::ValueTree sourceTree) const
    {
        if (treeHasMatchingTypeIgnoringCase(sourceTree, "ToggleButton"))
            return std::make_unique<juce::ToggleButton>();

        return nullptr;
    }
} // namespace jive
