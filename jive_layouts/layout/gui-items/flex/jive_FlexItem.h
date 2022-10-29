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
        TypedValue<int> flexItemOrder;
        TypedValue<float> flexItemGrow;
        TypedValue<float> flexItemShrink;
        TypedValue<float> flexItemBasis;
        TypedValue<juce::FlexItem::AlignSelf> flexItemAlignSelf;
        Length width;
        Length height;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlexItem)
    };
} // namespace jive
