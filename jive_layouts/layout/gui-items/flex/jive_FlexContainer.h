#pragma once

namespace jive
{
    class FlexContainer : public ContainerItem
    {
    public:
        explicit FlexContainer(std::unique_ptr<GuiItem> itemToDecorate);

        void layOutChildren() override;

        operator juce::FlexBox();

    protected:
        juce::Rectangle<float> calculateIdealSize(juce::Rectangle<float> constraints) const override;

    private:
        juce::FlexBox buildFlexBox(juce::Rectangle<float> bounds, LayoutStrategy strategy);

        Property<juce::FlexBox::Direction> flexDirection;
        Property<juce::FlexBox::Wrap> flexWrap;
        Property<juce::FlexBox::JustifyContent> flexJustifyContent;
        Property<juce::FlexBox::AlignItems> flexAlignItems;
        Property<juce::FlexBox::AlignContent> flexAlignContent;

        const BoxModel& boxModel;

        JUCE_LEAK_DETECTOR(FlexContainer)
    };
} // namespace jive
