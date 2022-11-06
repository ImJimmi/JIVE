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
        void addChild(std::unique_ptr<GuiItem> child) override;

        //==============================================================================================================
        void layOutChildren() override;

        //==============================================================================================================
        operator juce::Grid();

    private:
        //==============================================================================================================
        juce::Grid buildGrid();
        juce::Grid buildGridWithDummyItems() const;

        float calculateMinWidth() const;
        float calculateMinHeight() const;

        void layoutChanged();

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
        TypedValue<juce::Array<juce::Grid::Px>> gap;
        Length autoMinWidth;
        Length autoMinHeight;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridContainer)
    };
} // namespace jive
