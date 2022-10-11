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
        void updateLayout() override;

        //==============================================================================================================
        void addChild(std::unique_ptr<GuiItem> child) override;

        //==============================================================================================================
        operator juce::FlexBox();

    protected:
        //==============================================================================================================
        void componentMovedOrResized(juce::Component& componentThatWasMovedOrResized,
                                     bool wasMoved,
                                     bool wasResized) override;

    private:
        //==============================================================================================================
        juce::FlexBox getFlexBox();
        juce::FlexBox getFlexBoxWithDummyItems() const;

        float calculateMinimumContentWidth(juce::FlexBox flex) const;
        float calculateMinimumContentHeight(juce::FlexBox flex) const;

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
