#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class FlexContainer : public GuiItemDecorator
    {
    public:
        //==============================================================================================================
        explicit FlexContainer(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        void addChild(std::unique_ptr<GuiItem> child) override;

        float calculateAutoWidth() const override;
        float calculateAutoHeight() const override;

        //==============================================================================================================
        void layOutChildren() override;

        //==============================================================================================================
        operator juce::FlexBox();

    private:
        //==============================================================================================================
        juce::FlexBox buildFlexBox();
        juce::FlexBox buildFlexBoxWithDummyItems() const;

        void updateExplicitSize();

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
