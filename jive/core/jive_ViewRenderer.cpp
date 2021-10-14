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
    std::unique_ptr<GuiItem> ViewRenderer::renderView(juce::ValueTree tree)
    {
        auto guiItem = createGuiItem(tree);

        appendChildItems(*guiItem, tree);

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

    std::unique_ptr<GuiItem> ViewRenderer::createGuiItem(juce::ValueTree tree) const
    {
        return std::make_unique<GuiItem>(createComponent(tree), tree);
    }

    void ViewRenderer::appendChildItems(GuiItem& item, juce::ValueTree tree)
    {
        for (auto childTree : tree)
            item.addChild(renderView(childTree));
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
} // namespace jive
