#pragma once

namespace jive
{
    class FlexItem : public ContainerItem::Child
    {
    public:
        explicit FlexItem(std::unique_ptr<GuiItem> itemToDecorate);

        [[nodiscard]] juce::FlexItem toJuceFlexItem(juce::Rectangle<float> parentContentBounds,
                                                    LayoutStrategy strategy);

    private:
        Property<int> order;
        Property<float> flexGrow;
        Property<float> flexShrink;
        Property<float> flexBasis;
        Property<juce::FlexItem::AlignSelf> alignSelf;
    };
} // namespace jive
