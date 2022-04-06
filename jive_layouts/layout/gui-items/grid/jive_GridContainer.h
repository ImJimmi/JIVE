#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class GridContainer : public GuiItemDecorator
    {
    public:
        //==============================================================================================================
        explicit GridContainer(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        operator juce::Grid();

    private:
        //==============================================================================================================
        TypedValue<juce::Grid::JustifyItems> justifyItems;
        TypedValue<juce::Grid::AlignItems> alignItems;
        TypedValue<juce::Grid::JustifyContent> justifyContent;
        TypedValue<juce::Grid::AlignContent> alignContent;
        TypedValue<juce::Grid::AutoFlow> autoFlow;
        TypedValue<juce::Array<juce::Grid::TrackInfo>> templateColumns;
        TypedValue<juce::Array<juce::Grid::TrackInfo>> templateRows;
        TypedValue<juce::StringArray> templateAreas;
        TypedValue<juce::Grid::TrackInfo> autoRows;
        TypedValue<juce::Grid::TrackInfo> autoColumns;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridContainer)
    };
} // namespace jive
