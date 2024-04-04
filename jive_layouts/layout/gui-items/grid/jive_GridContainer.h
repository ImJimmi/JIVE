#pragma once

#include <jive_layouts/layout/gui-items/jive_ContainerItem.h>

namespace jive
{
    class GridContainer : public ContainerItem
    {
    public:
        explicit GridContainer(std::unique_ptr<GuiItem> itemToDecorate);

        void layOutChildren() override;

        operator juce::Grid();

    protected:
        juce::Rectangle<float> calculateIdealSize(juce::Rectangle<float> constraints) const override;

    private:
        juce::Grid buildGrid(juce::Rectangle<int> bounds,
                             LayoutStrategy strategy);

        Property<juce::Grid::JustifyItems> justifyItems;
        Property<juce::Grid::AlignItems> alignItems;
        Property<juce::Grid::JustifyContent> justifyContent;
        Property<juce::Grid::AlignContent> alignContent;
        Property<juce::Grid::AutoFlow> gridAutoFlow;
        Property<juce::Array<juce::Grid::TrackInfo>> gridTemplateColumns;
        Property<juce::Array<juce::Grid::TrackInfo>> gridTemplateRows;
        Property<juce::StringArray> gridTemplateAreas;
        Property<juce::Grid::TrackInfo> gridAutoRows;
        Property<juce::Grid::TrackInfo> gridAutoColumns;
        Property<juce::Array<juce::Grid::Px>> gap;

        bool layoutRecursionLock = false;
        bool changesDuringLayout = false;

        const BoxModel& boxModel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridContainer)
    };
} // namespace jive
