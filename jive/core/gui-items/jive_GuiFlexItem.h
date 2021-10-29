#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class GuiFlexItem : public GuiItem
    {
    public:
        //==============================================================================================================
        explicit GuiFlexItem(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        operator juce::FlexItem();

    protected:
        //==============================================================================================================
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& id) override;

    private:
        //==============================================================================================================
        void flexItemOrderChanged();
        void flexItemGrowChanged();
        void flexItemAlignSelfChanged();

        //==============================================================================================================
        std::unique_ptr<GuiItem> item;

        juce::CachedValue<int> flexItemOrder;
        juce::CachedValue<float> flexItemGrow;
        juce::CachedValue<juce::FlexItem::AlignSelf> flexItemAlignSelf;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(GuiFlexItem)
    };
} // namespace jive
