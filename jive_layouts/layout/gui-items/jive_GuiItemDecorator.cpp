#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GuiItemDecorator::GuiItemDecorator(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItem{ *itemToDecorate }
        , item{ std::move(itemToDecorate) }
    {
        if (auto* decorator = dynamic_cast<GuiItemDecorator*>(item.get()))
            decorator->owner = this;
    }

    //==================================================================================================================
    void GuiItemDecorator::updateLayout()
    {
        item->updateLayout();
    }

    void GuiItemDecorator::updateSize()
    {
        item->updateSize();
    }

    void GuiItemDecorator::updatePosition()
    {
        item->updatePosition();
    }

    void GuiItemDecorator::informContentChanged()
    {
        getTopLevelDecorator().contentChanged();
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
    GuiItem::Iterator GuiItemDecorator::begin()
    {
        return std::begin(*item);
    }

    const GuiItem::Iterator GuiItemDecorator::begin() const
    {
        return std::begin(*item);
    }

    GuiItem::Iterator GuiItemDecorator::end()
    {
        return std::end(*item);
    }

    const GuiItem::Iterator GuiItemDecorator::end() const
    {
        return std::end(*item);
    }

    //==================================================================================================================
    void GuiItemDecorator::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyChanged, const juce::Identifier& propertyID)
    {
        GuiItem::valueTreePropertyChanged(treeWhosePropertyChanged, propertyID);
        item->valueTreePropertyChanged(treeWhosePropertyChanged, propertyID);
    }

    void GuiItemDecorator::valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childTree)
    {
        GuiItem::valueTreeChildAdded(parentTree, childTree);
        item->valueTreeChildAdded(parentTree, childTree);
    }

    void GuiItemDecorator::valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childTree, int childIndex)
    {
        GuiItem::valueTreeChildRemoved(parentTree, childTree, childIndex);
        item->valueTreeChildRemoved(parentTree, childTree, childIndex);
    }

    void GuiItemDecorator::componentMovedOrResized(juce::Component& componentThatWasMovedOrResized,
                                                   bool wasMoved,
                                                   bool wasResized)
    {
        GuiItem::componentMovedOrResized(componentThatWasMovedOrResized, wasMoved, wasResized);
        item->componentMovedOrResized(componentThatWasMovedOrResized, wasMoved, wasResized);
    }

    //==================================================================================================================
    GuiItemDecorator& GuiItemDecorator::getTopLevelDecorator()
    {
        if (owner != nullptr)
            return owner->getTopLevelDecorator();

        return *this;
    }

    const GuiItemDecorator& GuiItemDecorator::getTopLevelDecorator() const
    {
        if (owner != nullptr)
            return owner->getTopLevelDecorator();

        return *this;
    }
} // namespace jive
