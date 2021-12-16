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
        void addChild(std::unique_ptr<GuiItem> child) override;
        int getNumChildren() const override;
        GuiItem& getChild(int index) override;

        //==============================================================================================================
        void updateLayout() override;

        //==============================================================================================================
        operator juce::FlexBox() override;

    protected:
        //==============================================================================================================
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& id) override;

    private:
        //==============================================================================================================
        void forceUpdateOfAllCachedValues();

        //==============================================================================================================
        juce::CachedValue<juce::FlexBox::Direction> flexDirection;
        juce::CachedValue<juce::FlexBox::Wrap> flexWrap;
        juce::CachedValue<juce::FlexBox::JustifyContent> flexJustifyContent;
        juce::CachedValue<juce::FlexBox::AlignItems> flexAlignItems;
        juce::CachedValue<juce::FlexBox::AlignContent> flexAlignContent;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(GuiFlexContainer)
    };
} // namespace jive
