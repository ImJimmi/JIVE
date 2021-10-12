#include "jive_GuiItem.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GuiItem::GuiItem(std::unique_ptr<juce::Component> comp)
        : component{ std::move(comp) }
    {
        jassert(component != nullptr);
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

    void GuiItem::setDisplay(Display newDisplay)
    {
        display = newDisplay;
    }
} // namespace jive
