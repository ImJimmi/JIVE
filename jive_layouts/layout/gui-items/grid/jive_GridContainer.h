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

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridContainer)
    };
} // namespace jive
