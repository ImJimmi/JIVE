#pragma once

#include <jive_layouts/layout/gui-items/jive_ContainerItem.h>

namespace jive
{
    class GridItem
        : public ContainerItem::Child
        , private juce::ValueTree::Listener
    {
    public:
        explicit GridItem(std::unique_ptr<GuiItem> itemToDecorate);
        ~GridItem();

        [[nodiscard]] juce::GridItem toJuceGridItem(juce::Rectangle<float> parentContentBounds,
                                                    LayoutStrategy strategy);

        void idealSizeChanged() final;

    private:
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) final;

        PassiveProperty<int> order;
        PassiveProperty<juce::GridItem::JustifySelf> justifySelf;
        PassiveProperty<juce::GridItem::AlignSelf> alignSelf;
        PassiveProperty<juce::GridItem::StartAndEndProperty> gridColumn;
        PassiveProperty<juce::GridItem::StartAndEndProperty> gridRow;
        PassiveProperty<juce::String> gridArea;
        PassiveProperty<float> idealWidth;
        const PassiveProperty<juce::Grid::JustifyItems> parentJustifyItems;
        const PassiveProperty<juce::Grid::AlignItems> parentAlignItems;

        const BoxModel& box{ boxModel(*this) };
        std::unordered_map<ItemCacheKey, juce::GridItem> cachedItems;
        std::uint64_t cachedItemsConstraintsVersion = 0;
    };
} // namespace jive
