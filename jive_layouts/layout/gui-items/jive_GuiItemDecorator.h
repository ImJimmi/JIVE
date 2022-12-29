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
        const GuiItem* getParent() const override;
        GuiItem* getParent() override;

        GuiItemDecorator& getTopLevelDecorator();
        const GuiItemDecorator& getTopLevelDecorator() const;

        template <typename ItemType>
        ItemType* toType()
        {
            if (auto* itemWithType = dynamic_cast<ItemType*>(this))
                return itemWithType;
            else if (auto* decoratedDecorator = dynamic_cast<GuiItemDecorator*>(item.get()))
                return decoratedDecorator->toType<ItemType>();

            return nullptr;
        }

        //==============================================================================================================
        void layOutChildren() override;

        //==============================================================================================================
        Iterator begin() override;
        const Iterator begin() const override;
        Iterator end() override;
        const Iterator end() const override;

        //==============================================================================================================
        const std::unique_ptr<GuiItem> item;

    private:
        //==============================================================================================================
        GuiItemDecorator* owner = nullptr;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(GuiItemDecorator)
    };
} // namespace jive
