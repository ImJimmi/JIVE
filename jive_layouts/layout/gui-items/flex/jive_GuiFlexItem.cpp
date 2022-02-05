#include "jive_GuiFlexItem.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GuiFlexItem::GuiFlexItem(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , flexItemOrder{ tree, "order" }
        , flexItemGrow{ tree, "flex-grow" }
        , flexItemShrink{ tree, "flex-shrink", 1 }
        , flexItemBasis{ tree, "flex-basis" }
        , flexItemAlignSelf{ tree, "align-self", juce::FlexItem::AlignSelf::autoAlign }
    {
        flexItemOrder.onValueChange = [this]() {
            updateParentLayout();
        };
        flexItemGrow.onValueChange = [this]() {
            updateParentLayout();
        };
        flexItemShrink.onValueChange = [this]() {
            updateParentLayout();
        };
        flexItemBasis.onValueChange = [this]() {
            updateParentLayout();
        };
        flexItemAlignSelf.onValueChange = [this]() {
            updateParentLayout();
        };
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
        flexItem.flexShrink = flexItemShrink;
        flexItem.flexBasis = flexItemBasis;
        flexItem.alignSelf = flexItemAlignSelf;
        flexItem.margin = boxModelToFlexMargin(getBoxModel().getMargin());

        return flexItem;
    }

    //==================================================================================================================
    void GuiFlexItem::updateParentLayout()
    {
        if (auto* container = dynamic_cast<GuiFlexContainer*>(getParent()))
            container->updateLayout();
        else if (getParent() != nullptr)
            jassertfalse;
    }
} // namespace jive
