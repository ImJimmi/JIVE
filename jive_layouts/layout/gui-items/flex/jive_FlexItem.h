#pragma once

namespace jive
{
    class FlexItem : public GuiItemDecorator
    {
    public:
        explicit FlexItem(std::unique_ptr<GuiItem> itemToDecorate);

        juce::FlexItem toJuceFlexItem(juce::Rectangle<float> parentContentBounds,
                                      LayoutStrategy strategy) const;

    private:
        Property<int> order;
        Property<float> flexGrow;
        Property<float> flexShrink;
        Property<float> flexBasis;
        Property<juce::FlexItem::AlignSelf> alignSelf;
        const Length width;
        const Length height;
        const Length minWidth;
        const Length minHeight;
        const Property<float> idealWidth;
        const Property<float> idealHeight;

        const BoxModel& boxModel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlexItem)
    };
} // namespace jive
