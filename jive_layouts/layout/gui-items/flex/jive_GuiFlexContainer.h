#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class GuiFlexContainer : public GuiItemDecorator
    {
    public:
        //==============================================================================================================
        explicit GuiFlexContainer(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        float getHeight() const override;

        //==============================================================================================================
        void updateLayout() override;

        //==============================================================================================================
        operator juce::FlexBox();

    private:
        //==============================================================================================================
        juce::FlexBox getFlexBox();
        juce::FlexBox getFlexBoxWithDummyItems() const;

        float getMinimumContentHeight() const;

        //==============================================================================================================
        TypedValue<juce::FlexBox::Direction> flexDirection;
        TypedValue<juce::FlexBox::Wrap> flexWrap;
        TypedValue<juce::FlexBox::JustifyContent> flexJustifyContent;
        TypedValue<juce::FlexBox::AlignItems> flexAlignItems;
        TypedValue<juce::FlexBox::AlignContent> flexAlignContent;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(GuiFlexContainer)
    };
} // namespace jive
