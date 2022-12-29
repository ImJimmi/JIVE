#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class FlexContainer : public ContainerItem
    {
    public:
        //==============================================================================================================
        explicit FlexContainer(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        void layOutChildren() override;

        //==============================================================================================================
        operator juce::FlexBox();

    protected:
        //==============================================================================================================
        juce::Rectangle<float> calculateIdealSize(juce::Rectangle<float> constraints) const override;

    private:
        //==============================================================================================================
        juce::FlexBox buildFlexBox(juce::Rectangle<float> bounds, LayoutStrategy strategy);

        //==============================================================================================================
        TypedValue<juce::FlexBox::Direction> flexDirection;
        TypedValue<juce::FlexBox::Wrap> flexWrap;
        TypedValue<juce::FlexBox::JustifyContent> flexJustifyContent;
        TypedValue<juce::FlexBox::AlignItems> flexAlignItems;
        TypedValue<juce::FlexBox::AlignContent> flexAlignContent;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(FlexContainer)
    };
} // namespace jive
