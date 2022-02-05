#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class GuiItemDecorator : public GuiItem
    {
    public:
        //==============================================================================================================
        explicit GuiItemDecorator(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        void addChild(std::unique_ptr<GuiItem> child) override;
        int getNumChildren() const override;
        GuiItem& getChild(int index) const override;

        float getWidth() const override;
        float getHeight() const override;

        operator juce::FlexBox() const override;
        operator juce::FlexItem() override;

    protected:
        //==============================================================================================================
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& id) override;
        void valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& child) override;
        void valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& child, int childIndex) override;

        void componentMovedOrResized(juce::Component& component, bool wasMoved, bool wasResized) override;

        //==============================================================================================================
        const std::unique_ptr<GuiItem> item;

    private:
        //==============================================================================================================
        JUCE_LEAK_DETECTOR(GuiItemDecorator)
    };
} // namespace jive
