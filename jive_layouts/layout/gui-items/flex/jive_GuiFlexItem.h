#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class GuiFlexItem : public GuiItemDecorator
    {
    public:
        //==============================================================================================================
        explicit GuiFlexItem(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        operator juce::FlexItem() override;

    protected:
        //==============================================================================================================
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& id) override;

    private:
        //==============================================================================================================
        void forceUpdateOfAllCachedValues();
        void updateParentLayout();

        //==============================================================================================================
        juce::CachedValue<int> flexItemOrder;
        juce::CachedValue<float> flexItemGrow;
        juce::CachedValue<float> flexItemShrink;
        juce::CachedValue<juce::FlexItem::AlignSelf> flexItemAlignSelf;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(GuiFlexItem)
    };
} // namespace jive
