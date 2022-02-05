#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class GuiFlexItem : public GuiItemDecorator
    {
    public:
        //==============================================================================================================
        explicit GuiFlexItem(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        operator juce::FlexItem() override;

    private:
        //==============================================================================================================
        void updateParentLayout();

        //==============================================================================================================
        TypedValue<int> flexItemOrder;
        TypedValue<float> flexItemGrow;
        TypedValue<float> flexItemShrink;
        TypedValue<float> flexItemBasis;
        TypedValue<juce::FlexItem::AlignSelf> flexItemAlignSelf;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(GuiFlexItem)
    };
} // namespace jive
