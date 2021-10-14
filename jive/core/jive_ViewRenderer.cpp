#include "jive_ViewRenderer.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    ViewRenderer::ViewRenderer()
    {
        resetFactories();
    }

    //==================================================================================================================
    std::unique_ptr<GuiItem> ViewRenderer::renderView(juce::ValueTree tree)
    {
        auto guiItem = createGuiItem(tree);

        appendChildItems(*guiItem, tree);

        return guiItem;
    }

    //==================================================================================================================
    void ViewRenderer::setFactory (const juce::Identifier& treeType, ComponentFactory factory)
    {
        factories.set(treeType.toString(), factory);
    }

    void ViewRenderer::resetFactories()
    {
        factories.clear();

        setFactory("TextButton", []() { return std::make_unique<juce::TextButton>(); });
        setFactory("ToggleButton", []() { return std::make_unique<juce::ToggleButton>(); });
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

        if (factories.contains(treeType))
        {
            const auto create = factories[treeType];
            return create();
        }

        // No creator for the given ValueTree's type.
        jassertfalse;
        return nullptr;
    }
} // namespace jive
