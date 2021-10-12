#include "jive_ViewRenderer.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    juce::Component* ViewRenderer::createView(juce::ValueTree tree)
    {
        if (auto* component = createAndAddComponent(tree))
        {
            applyProperties(tree, *component);
            createAndAddChildren(tree, *component);

            return component;
        }

        return nullptr;
    }

    //==================================================================================================================
    bool treeHasMatchingTypeIgnoringCase(const juce::ValueTree& tree, const juce::String& expectedType)
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

    juce::Component* ViewRenderer::createAndAddComponent(juce::ValueTree tree)
    {
        if (auto component = createComponentForJiveTree(tree))
            return components.add(std::move(component));

        return nullptr;
    }

    void ViewRenderer::applyProperties(juce::ValueTree tree, juce::Component& component)
    {
        component.setComponentID(tree["id"]);
    }

    void ViewRenderer::createAndAddChildren(juce::ValueTree tree, juce::Component& component)
    {
        for (const auto& childTree : tree)
        {
            if (auto childComponent = createView(childTree))
                component.addChildComponent(childComponent);
        }
    }
} // namespace jive
