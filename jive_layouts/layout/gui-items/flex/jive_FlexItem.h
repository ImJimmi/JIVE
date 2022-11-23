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
        juce::FlexItem toJuceFlexItem(juce::Rectangle<float> parentContentBounds,
                                      LayoutStrategy strategy) const;

    private:
        //==============================================================================================================
        TypedValue<int> flexItemOrder;
        TypedValue<float> flexItemGrow;
        TypedValue<float> flexItemShrink;
        TypedValue<float> flexItemBasis;
        TypedValue<juce::FlexItem::AlignSelf> flexItemAlignSelf;
        const Length width;
        const Length height;
        const Length minWidth;
        const Length minHeight;
        const TypedValue<float> idealWidth;
        const TypedValue<float> idealHeight;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlexItem)
    };
} // namespace jive
