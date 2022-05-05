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
        void updateLayout() override;

        //==============================================================================================================
        void addChild(std::unique_ptr<GuiItem> child) override;
        int getNumChildren() const override;
        GuiItem& getChild(int index) const override;

        template <typename ItemType>
        ItemType* toType()
        {
            if (auto* itemWithType = dynamic_cast<ItemType*>(this))
                return itemWithType;
            else if (auto* decoratedDecorator = dynamic_cast<GuiItemDecorator*>(item.get()))
                return decoratedDecorator->toType<ItemType>();

            return nullptr;
        }

        float getWidth() const override;
        float getHeight() const override;

        //==============================================================================================================
        Iterator begin() override;
        const Iterator begin() const override;
        Iterator end() override;
        const Iterator end() const override;

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
