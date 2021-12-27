#include "jive_GuiFlexItem.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GuiFlexItem::GuiFlexItem(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , flexItemOrder{ tree, "order", nullptr }
        , flexItemGrow{ tree, "flex-grow", nullptr }
        , flexItemAlignSelf{ tree, "align-self", nullptr, juce::FlexItem::AlignSelf::autoAlign }
    {
    }

    //==================================================================================================================
    juce::FlexItem::Margin boxModelToFlexMargin(const juce::BorderSize<float>& border)
    {
        return juce::FlexItem::Margin{
            border.getTop(),
            border.getRight(),
            border.getBottom(),
            border.getLeft()
        };
    }

    GuiFlexItem::operator juce::FlexItem()
    {
        juce::FlexItem flexItem{ *component };

        flexItem.width = getWidth();
        flexItem.height = getHeight();

        flexItem.order = flexItemOrder;
        flexItem.flexGrow = flexItemGrow;
        flexItem.alignSelf = flexItemAlignSelf;
        flexItem.margin = boxModelToFlexMargin(getBoxModel().getMargin());

        return flexItem;
    }

    //==================================================================================================================
    void GuiFlexItem::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyChanged,
                                               const juce::Identifier& propertyID)
    {
        GuiItemDecorator::valueTreePropertyChanged(treeWhosePropertyChanged, propertyID);

        if (treeWhosePropertyChanged != tree)
            return;

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
        else if (getParent() != nullptr)
            jassertfalse;
    }
} // namespace jive
