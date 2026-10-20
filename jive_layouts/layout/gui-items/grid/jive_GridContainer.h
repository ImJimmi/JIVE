#pragma once

#include <jive_layouts/layout/gui-items/jive_ContainerItem.h>

namespace jive
{
    class GridContainer
        : public ContainerItem
    {
    public:
        explicit GridContainer(std::unique_ptr<GuiItem> itemToDecorate);
        ~GridContainer() override;

        void layOutChildren() override;

        explicit operator juce::Grid();

    protected:
        juce::Rectangle<float> calculateIdealSize(juce::Rectangle<float> constraints) const override;

    private:
        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) final;

        juce::Grid buildGrid(juce::Rectangle<int> bounds,
                             LayoutStrategy strategy);

        PassiveProperty<juce::Grid::JustifyItems> justifyItems;
        PassiveProperty<juce::Grid::AlignItems> alignItems;
        PassiveProperty<juce::Grid::JustifyContent> justifyContent;
        PassiveProperty<juce::Grid::AlignContent> alignContent;
        PassiveProperty<juce::Grid::AutoFlow> gridAutoFlow;
        PassiveProperty<juce::Array<juce::Grid::TrackInfo>> gridTemplateColumns;
        PassiveProperty<juce::Array<juce::Grid::TrackInfo>> gridTemplateRows;
        PassiveProperty<juce::StringArray> gridTemplateAreas;
        PassiveProperty<juce::Grid::TrackInfo> gridAutoRows;
        PassiveProperty<juce::Grid::TrackInfo> gridAutoColumns;
        PassiveProperty<juce::Array<juce::Grid::Px>> gap;

        bool layoutRecursionLock = false;
        bool changesDuringLayout = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridContainer)
    };
} // namespace jive
