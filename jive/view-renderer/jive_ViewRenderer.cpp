#include "jive_ViewRenderer.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    bool treeHasMatchingTypeIgnoringCase (const juce::ValueTree& tree, const juce::String& expectedType)
    {
        return tree.getType().toString().equalsIgnoreCase(expectedType);
    }

    std::unique_ptr<juce::Component> createComponentForJiveTree(const juce::ValueTree& tree)
    {
        if (treeHasMatchingTypeIgnoringCase(tree, "ToggleButton"))
            return std::make_unique<juce::ToggleButton>();

        if (treeHasMatchingTypeIgnoringCase(tree, "TextButton"))
            return std::make_unique<juce::TextButton>();

        return nullptr;
    }

    std::unique_ptr<juce::Component> ViewRenderer::createView(juce::ValueTree sourceTree) const
    {
        auto component = createComponentForJiveTree(sourceTree);

        if (component != nullptr)
        {
            component->setComponentID(sourceTree["id"]);
        }

        return component;
    }
} // namespace jive
