#include "jive_GuiFlexItem.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GuiFlexItem::GuiFlexItem(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItem{ *itemToDecorate }
        , item{ std::move(itemToDecorate) }
        , flexItemOrder{ tree, "order", nullptr }
        , flexItemGrow{ tree, "flex-grow", nullptr }
        , flexItemAlignSelf{ tree, "align-self", nullptr, juce::FlexItem::AlignSelf::autoAlign }
    {
    }

    //==================================================================================================================
    GuiFlexItem::operator juce::FlexItem()
    {
        juce::FlexItem flexItem{ *component };

        flexItem.width = getWidth();
        flexItem.height = getHeight();

        flexItem.order = flexItemOrder;
        flexItem.flexGrow = flexItemGrow;
        flexItem.alignSelf = flexItemAlignSelf;

        return flexItem;
    }

    //==================================================================================================================
    void GuiFlexItem::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyChanged,
                                               const juce::Identifier& propertyID)
    {
        jassert(treeWhosePropertyChanged == tree);

        item->valueTreePropertyChanged(treeWhosePropertyChanged, propertyID);

        forceUpdateOfAllCachedValues();
        updateParentLayout();
    }

    //==================================================================================================================
    void GuiFlexItem::forceUpdateOfAllCachedValues()
    {
        flexItemOrder.forceUpdateOfCachedValue();
        flexItemGrow.forceUpdateOfCachedValue();
        flexItemAlignSelf.forceUpdateOfCachedValue();
    }

    void GuiFlexItem::updateParentLayout()
    {
        if (auto* container = dynamic_cast<GuiFlexContainer*>(getParent()))
            container->updateLayout();
        else
            jassertfalse;
    }
} // namespace jive
