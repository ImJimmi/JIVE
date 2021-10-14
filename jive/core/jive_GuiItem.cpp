#include "jive_GuiItem.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GuiItem::GuiItem(std::unique_ptr<juce::Component> comp, juce::ValueTree valueTree)
        : component{ std::move(comp) }
        , tree{ valueTree }
        , display{ tree, "display", nullptr, Display::flex }
    {
        jassert(component != nullptr);

        componentIdChanged();

        tree.addListener(this);
    }

    //==================================================================================================================
    void GuiItem::addChild(std::unique_ptr<GuiItem> child)
    {
        auto* newlyAddedChild = children.add(std::move(child));
        component->addAndMakeVisible(newlyAddedChild->getComponent());
    }

    //==================================================================================================================
    juce::Component& GuiItem::getComponent()
    {
        return *component;
    }

    const juce::Component& GuiItem::getComponent() const
    {
        return *component;
    }

    int GuiItem::getNumChildren() const
    {
        return children.size();
    }

    GuiItem& GuiItem::getChild(int index)
    {
        jassert(index < children.size());
        return *children[index];
    }

    juce::String GuiItem::getID() const
    {
        return component->getComponentID();
    }

    GuiItem::Display GuiItem::getDisplay() const
    {
        return display;
    }

    //==================================================================================================================
    void GuiItem::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyChanged,
                                           const juce::Identifier& propertyID)
    {
        jassert(treeWhosePropertyChanged == tree);

        if (propertyID == juce::Identifier{ "id" })
            componentIdChanged();
    }

    //==================================================================================================================
    void GuiItem::componentIdChanged()
    {
        component->setComponentID(tree["id"]);
    }
} // namespace jive
