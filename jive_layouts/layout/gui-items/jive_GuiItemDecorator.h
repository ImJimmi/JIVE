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
        void updateSize() override;
        void updatePosition() override;
        void informContentChanged() override;

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

        //==============================================================================================================
        Iterator begin() override;
        const Iterator begin() const override;
        Iterator end() override;
        const Iterator end() const override;

    protected:
        //==============================================================================================================
        GuiItemDecorator& getTopLevelDecorator();
        const GuiItemDecorator& getTopLevelDecorator() const;

        //==============================================================================================================
        const std::unique_ptr<GuiItem> item;

    private:
        //==============================================================================================================
        GuiItemDecorator* owner = nullptr;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(GuiItemDecorator)
    };
} // namespace jive
