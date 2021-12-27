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
    std::unique_ptr<GuiItem> ViewRenderer::renderView(juce::ValueTree tree) const
    {
        // Can't render a view from an invalid tree!
        jassert(tree.isValid());

        return renderView(tree, nullptr);
    }

    void replaceTextElementWithTextProperty(juce::XmlElement& parentXML, juce::XmlElement& textChild)
    {
        parentXML.setAttribute("text", textChild.getText());
        parentXML.removeChildElement(&textChild, true);
    }

    void recursivelyReplaceSubTextElementsWithTextProperties(juce::XmlElement& xml)
    {
        for (auto i = xml.getNumChildElements() - 1; i >= 0; i--)
        {
            auto* child = xml.getChildElement(i);

            recursivelyReplaceSubTextElementsWithTextProperties(*child);

            if (child->getTagName().isEmpty())
                replaceTextElementWithTextProperty(xml, *child);
        }
    }

    std::unique_ptr<GuiItem> ViewRenderer::renderView(const juce::String& xmlString) const
    {
        auto xml = juce::parseXML(xmlString);
        recursivelyReplaceSubTextElementsWithTextProperties(*xml);

        return renderView(juce::ValueTree::fromXml(*xml));
    }

    //==================================================================================================================
    void ViewRenderer::setFactory(const juce::Identifier& treeType, ComponentFactory factory)
    {
        factories.set(treeType.toString(), factory);
    }

    void ViewRenderer::resetFactories()
    {
        factories.clear();

        setFactory("ComboBox", []() {
            return std::make_unique<juce::ComboBox>();
        });
        setFactory("Component", []() {
            return std::make_unique<juce::Component>();
        });
        setFactory("Label", []() {
            return std::make_unique<juce::Label>();
        });
        setFactory("TextButton", []() {
            return std::make_unique<juce::TextButton>();
        });
        setFactory("ToggleButton", []() {
            return std::make_unique<juce::ToggleButton>();
        });
    }

    //==================================================================================================================
    std::unique_ptr<GuiItem> decorateWithDisplayBehaviour(std::unique_ptr<GuiItem> item)
    {
        switch (item->getDisplay())
        {
        case GuiItem::Display::flex:
            return std::make_unique<GuiFlexContainer>(std::move(item));
        }
    }

    std::unique_ptr<GuiItem> decorateWithHereditaryBehaviour(std::unique_ptr<GuiItem> item)
    {
        if (item->getParent() == nullptr)
            return item;

        switch (item->getParent()->getDisplay())
        {
        case GuiItem::Display::flex:
            return std::make_unique<GuiFlexItem>(std::move(item));
        }
    }

    std::unique_ptr<GuiItem> decorateWithWidgetBehaviour(std::unique_ptr<GuiItem> item, const juce::ValueTree& tree)
    {
        if (tree.hasType("ComboBox"))
            return std::make_unique<ComboBox>(std::move(item));
        if (tree.hasType("Label"))
            return std::make_unique<Label>(std::move(item));

        return item;
    }

    std::unique_ptr<GuiItem> ViewRenderer::renderView(juce::ValueTree tree, GuiItem* const parent) const
    {
        auto guiItem = createGuiItem(tree, parent);

        guiItem = decorateWithDisplayBehaviour(std::move(guiItem));
        guiItem = decorateWithWidgetBehaviour(std::move(guiItem), tree);
        guiItem = decorateWithHereditaryBehaviour(std::move(guiItem));

        if (guiItem->isContainer())
            appendChildItems(*guiItem, tree);

        return guiItem;
    }

    bool treeHasMatchingTypeIgnoringCase(const juce::ValueTree& tree, const juce::String& expectedType)
    {
        return tree.getType().toString().equalsIgnoreCase(expectedType);
    }

    std::unique_ptr<GuiItem> ViewRenderer::createGuiItem(juce::ValueTree tree, GuiItem* const parent) const
    {
        return std::make_unique<GuiItem>(createComponent(tree), tree, parent);
    }

    void ViewRenderer::appendChildItems(GuiItem& item, juce::ValueTree tree) const
    {
        for (auto childTree : tree)
            item.addChild(renderView(childTree, &item));
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
