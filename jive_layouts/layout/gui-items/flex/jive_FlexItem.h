#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class FlexItem : public GuiItemDecorator
    {
    public:
        //==============================================================================================================
        explicit FlexItem(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        operator juce::FlexItem();

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
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlexItem)
    };
} // namespace jive
