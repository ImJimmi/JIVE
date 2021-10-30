#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class GuiFlexContainer : public GuiItem
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
        operator juce::FlexBox();

    protected:
        //==============================================================================================================
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& id) override;
        void componentMovedOrResized(juce::Component& component, bool wasMoved, bool wasResized) override;

    private:
        //==============================================================================================================
        void flexDirectionChanged();
        void flexWrapChanged();
        void flexJustifyContentChanged();
        void flexAlignItemsChanged();
        void flexAlignContentChanged();

        //==============================================================================================================
        const std::unique_ptr<GuiItem> item;

        juce::CachedValue<juce::FlexBox::Direction> flexDirection;
        juce::CachedValue<juce::FlexBox::Wrap> flexWrap;
        juce::CachedValue<juce::FlexBox::JustifyContent> flexJustifyContent;
        juce::CachedValue<juce::FlexBox::AlignItems> flexAlignItems;
        juce::CachedValue<juce::FlexBox::AlignContent> flexAlignContent;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(GuiFlexContainer)
    };
} // namespace jive
