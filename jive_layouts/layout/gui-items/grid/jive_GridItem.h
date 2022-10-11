#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class GridItem : public GuiItemDecorator
    {
    public:
        //==============================================================================================================
        explicit GridItem(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        operator juce::GridItem();

    private:
        //==============================================================================================================
        TypedValue<int> order;
        TypedValue<juce::GridItem::JustifySelf> justifySelf;
        TypedValue<juce::GridItem::AlignSelf> alignSelf;
        TypedValue<juce::GridItem::StartAndEndProperty> column;
        TypedValue<juce::GridItem::StartAndEndProperty> row;
        TypedValue<juce::String> area;
        TypedValue<float> minWidth;
        TypedValue<float> maxWidth;
        TypedValue<float> minHeight;
        TypedValue<float> maxHeight;
        Length width;
        Length height;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridItem)
    };
} // namespace jive
