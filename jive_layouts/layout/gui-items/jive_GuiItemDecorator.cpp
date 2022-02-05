#include "jive_GuiItemDecorator.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GuiItemDecorator::GuiItemDecorator(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItem{ *itemToDecorate }
        , item{ std::move(itemToDecorate) }
    {
    }

    //==================================================================================================================
    void GuiItemDecorator::addChild(std::unique_ptr<GuiItem> child)
    {
        item->addChild(std::move(child));
    }

    int GuiItemDecorator::getNumChildren() const
    {
        return item->getNumChildren();
    }

    GuiItem& GuiItemDecorator::getChild(int index) const
    {
        return item->getChild(index);
    }

    float GuiItemDecorator::getWidth() const
    {
        return item->getWidth();
    }

    float GuiItemDecorator::getHeight() const
    {
        return item->getHeight();
    }

    //==================================================================================================================
    void GuiItemDecorator::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& id)
    {
        GuiItem::valueTreePropertyChanged(tree, id);
        item->valueTreePropertyChanged(tree, id);
    }

    void GuiItemDecorator::valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& child)
    {
        GuiItem::valueTreeChildAdded(parent, child);
        item->valueTreeChildAdded(parent, child);
    }

    void GuiItemDecorator::valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& child, int childIndex)
    {
        GuiItem::valueTreeChildRemoved(parent, child, childIndex);
        item->valueTreeChildRemoved(parent, child, childIndex);
    }

    void GuiItemDecorator::componentMovedOrResized(juce::Component& component, bool wasMoved, bool wasResized)
    {
        GuiItem::componentMovedOrResized(component, wasMoved, wasResized);
        item->componentMovedOrResized(component, wasMoved, wasResized);
    }
} // namespace jive
