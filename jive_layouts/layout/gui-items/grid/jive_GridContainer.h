#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class GridContainer : public ContainerItem
    {
    public:
        //==============================================================================================================
        explicit GridContainer(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        void layOutChildren() override;

        //==============================================================================================================
        operator juce::Grid();

    protected:
        //==============================================================================================================
        juce::Rectangle<float> calculateIdealSize(juce::Rectangle<float> constraints) const override;

    private:
        //==============================================================================================================
        juce::Grid buildGrid();
        juce::Grid buildGridWithDummyItems() const;

        //==============================================================================================================
        Property<juce::Grid::JustifyItems> justifyItems;
        Property<juce::Grid::AlignItems> alignItems;
        Property<juce::Grid::JustifyContent> justifyContent;
        Property<juce::Grid::AlignContent> alignContent;
        Property<juce::Grid::AutoFlow> autoFlow;
        Property<juce::Array<juce::Grid::TrackInfo>> templateColumns;
        Property<juce::Array<juce::Grid::TrackInfo>> templateRows;
        Property<juce::StringArray> templateAreas;
        Property<juce::Grid::TrackInfo> autoRows;
        Property<juce::Grid::TrackInfo> autoColumns;
        Property<juce::Array<juce::Grid::Px>> gap;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridContainer)
    };
} // namespace jive
