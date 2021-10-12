#include "jive_ViewRenderer.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    ViewRenderer::ViewRenderer()
    {
        resetComponentCreators();
    }

    //==================================================================================================================
    juce::Component& ViewRenderer::renderView(juce::ValueTree tree)
    {
        auto& component = addComponent(tree);

        applyProperties(tree, component);
        createAndAddChildren(tree, component);

        return component;
    }

    //==================================================================================================================
    void ViewRenderer::setComponentCreator (const juce::String& customComponentType, ComponentCreator creator)
    {
        componentCreators.set(customComponentType, creator);
    }

    void ViewRenderer::resetComponentCreators()
    {
        componentCreators.clear();

        setComponentCreator("TextButton", []() { return std::make_unique<juce::TextButton>(); });
        setComponentCreator("ToggleButton", []() { return std::make_unique<juce::ToggleButton>(); });
    }

    //==================================================================================================================
    bool treeHasMatchingTypeIgnoringCase(const juce::ValueTree& tree, const juce::String& expectedType)
    {
        return tree.getType().toString().equalsIgnoreCase(expectedType);
    }

    juce::Component& ViewRenderer::addComponent(juce::ValueTree tree)
    {
        if (auto component = createComponent(tree))
            return *components.add(std::move(component));

        // Failed to create a component for the given tree.
        jassertfalse;
        return *components[0];
    }

    std::unique_ptr<juce::Component> ViewRenderer::createComponent(juce::ValueTree tree) const
    {
        const auto treeType = tree.getType().toString();

        if (componentCreators.contains(treeType))
        {
            const auto creator = componentCreators[treeType];
            return creator();
        }

        // No creator for the given ValueTree's type.
        jassertfalse;
        return nullptr;
    }

    void ViewRenderer::applyProperties(juce::ValueTree tree, juce::Component& component) const
    {
        component.setComponentID(tree["id"]);
    }

    void ViewRenderer::createAndAddChildren(juce::ValueTree tree, juce::Component& component)
    {
        for (const auto& childTree : tree)
        {
            auto& childComponent = renderView(childTree);
            component.addChildComponent(childComponent);
        }
    }
} // namespace jive
