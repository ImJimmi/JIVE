#pragma once

#include <jive_layouts/layout/gui-items/jive_ContainerItem.h>

namespace jive
{
    class FlexItem
        : public ContainerItem::Child
        , private juce::ValueTree::Listener
    {
    public:
        explicit FlexItem(std::unique_ptr<GuiItem> itemToDecorate);
        ~FlexItem();

        [[nodiscard]] juce::FlexItem toJuceFlexItem(juce::Rectangle<float> parentContentBounds,
                                                    LayoutStrategy strategy);

        void idealSizeChanged() final;

    private:
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) final;

        PassiveProperty<int> order;
        PassiveProperty<float> flexGrow;
        PassiveProperty<float> flexShrink;
        PassiveProperty<float> flexBasis;
        PassiveProperty<juce::FlexItem::AlignSelf> alignSelf;
        const PassiveProperty<juce::FlexBox::Direction> parentDirection;

        const BoxModel& box{ boxModel(*this) };
        const std::unique_ptr<juce::Component> layoutDummy;
        std::unordered_map<ItemCacheKey, juce::FlexItem> cachedItems;
        std::uint64_t cachedItemsConstraintsVersion = 0;
        Orientation cachedItemsOrientation = Orientation::vertical;
        float lastMinHeight = -1.0f;
    };
} // namespace jive
