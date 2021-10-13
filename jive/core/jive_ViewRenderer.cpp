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
    GuiItem& ViewRenderer::renderView(juce::ValueTree tree)
    {
        auto& guiItem = addGuiItem(tree);

        createAndAddChildren(tree, guiItem);

        return guiItem;
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

    GuiItem& ViewRenderer::addGuiItem(juce::ValueTree tree)
    {
        if (auto component = createComponent(tree))
            return *guiItems.add(std::make_unique<GuiItem>(std::move(component), tree));

        // Failed to create a component for the given tree.
        jassertfalse;
        return *guiItems[0];
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

    void ViewRenderer::createAndAddChildren(juce::ValueTree tree, GuiItem& guiItem)
    {
        for (const auto& childTree : tree)
        {
            auto& childComponent = renderView(childTree);
            guiItem.getComponent().addChildComponent(childComponent.getComponent());
        }
    }
} // namespace jive
