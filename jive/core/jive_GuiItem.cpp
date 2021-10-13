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
    juce::Component& GuiItem::getComponent()
    {
        return *component;
    }

    const juce::Component& GuiItem::getComponent() const
    {
        return *component;
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
