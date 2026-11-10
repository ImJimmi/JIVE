#pragma once

#include <jive_layouts/layout/gui-items/jive_ContainerItem.h>

namespace jive
{
    using FlexGrow = Number<float, struct FlexGrowTag>;
    using FlexShrink = Number<float, struct FlexShrinkTag>;
    using FlexBasis = Number<float, struct FlexBasisTag>;

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

        PassiveProperty<Order> order;
        PassiveProperty<FlexGrow> flexGrow;
        PassiveProperty<FlexShrink> flexShrink;
        PassiveProperty<FlexBasis> flexBasis;
        PassiveProperty<juce::FlexItem::AlignSelf> alignSelf;
        const PassiveProperty<juce::FlexBox::Direction> parentDirection;
        const PassiveProperty<juce::FlexBox::AlignItems> parentAlignItems;

        const BoxModel& box{ boxModel(*this) };
        const std::unique_ptr<juce::Component> layoutDummy;
        std::unordered_map<ItemCacheKey, juce::FlexItem> cachedItems;
        std::uint64_t cachedItemsConstraintsVersion = 0;
        Orientation cachedItemsOrientation = Orientation::vertical;
        float lastMinHeight = -1.0f;
    };
} // namespace jive
