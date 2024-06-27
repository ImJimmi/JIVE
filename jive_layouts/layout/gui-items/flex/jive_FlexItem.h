#pragma once

#include <jive_layouts/layout/gui-items/jive_ContainerItem.h>

namespace std
{
    template <>
    struct hash<std::pair<juce::Rectangle<float>, jive::LayoutStrategy>>
    {
        [[nodiscard]] std::size_t operator()(const std::pair<juce::Rectangle<float>, jive::LayoutStrategy>& pair) const noexcept
        {
            return (pair.first.toString() + jive::toString(pair.second)).hash();
        }
    };
} // namespace std

namespace jive
{
    class FlexItem
        : public ContainerItem::Child
        , private BoxModel::Listener
    {
    public:
        explicit FlexItem(std::unique_ptr<GuiItem> itemToDecorate);
        ~FlexItem();

        [[nodiscard]] juce::FlexItem toJuceFlexItem(juce::Rectangle<float> parentContentBounds,
                                                    LayoutStrategy strategy);

    private:
        void boxModelInvalidated(BoxModel&) final;

        Property<int> order;
        Property<float> flexGrow;
        Property<float> flexShrink;
        Property<float> flexBasis;
        Property<juce::FlexItem::AlignSelf> alignSelf;

        const BoxModel& box{ boxModel(*this) };
        const std::unique_ptr<juce::Component> layoutDummy;
        std::unordered_map<std::pair<juce::Rectangle<float>, LayoutStrategy>, juce::FlexItem> cachedItems;
    };
} // namespace jive
