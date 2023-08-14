#pragma once

namespace jive
{
    class FlexContainer
        : public ContainerItem
        , private juce::ValueTree::Listener
    {
    public:
        explicit FlexContainer(std::unique_ptr<GuiItem> itemToDecorate);
        ~FlexContainer() override;

        void layOutChildren() override;

        operator juce::FlexBox();

    protected:
        juce::Rectangle<float> calculateIdealSize(juce::Rectangle<float> constraints) const override;

    private:
        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) final;

        juce::FlexBox buildFlexBox(juce::Rectangle<float> bounds, LayoutStrategy strategy);

        Property<juce::FlexBox::Direction> flexDirection;
        Property<juce::FlexBox::Wrap> flexWrap;
        Property<juce::FlexBox::JustifyContent> flexJustifyContent;
        Property<juce::FlexBox::AlignItems> flexAlignItems;
        Property<juce::FlexBox::AlignContent> flexAlignContent;

        bool layoutRecursionLock = false;
        bool changesDuringLayout = false;

        const BoxModel& boxModel;

        JUCE_LEAK_DETECTOR(FlexContainer)
    };
} // namespace jive
